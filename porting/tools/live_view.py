#!/usr/bin/env python3
"""Live MJPEG preview of the original client's Xvfb display(s) in a browser.

    python3 -I porting/tools/live_view.py [--host 0.0.0.0] [--port 8766]

Pages:
  /                      index: one live view per running Xvfb display (gallery 1 fps, opened view 24 fps)
  /api/displays          JSON list of displays found now
  /stream/<N>?fps=&scale=  multipart/x-mixed-replace MJPEG of display :N (default 24 fps, native size)

Displays are found on every request from `ps -eo args` (Xvfb :N ... -auth FILE). One ffmpeg x11grab runs per
(display, fps, scale) only while at least one HTTP client watches it, shared between those clients, and is
stopped a few seconds after the last one leaves. No frame is ever written to disk.
"""
import argparse
import json
import os
import re
import signal
import subprocess
import sys
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import parse_qs, urlparse

BOUNDARY = b"frame"
IDLE_STOP_SECONDS = 3.0
FRAME_WAIT_SECONDS = 10.0
MAX_FPS = 30
XVFB_RE = re.compile(r"Xvfb\s+(:\d+)(.*)$")


def log(msg):
    sys.stderr.write(time.strftime("%Y-%m-%d %H:%M:%S ") + msg + "\n")
    sys.stderr.flush()


def scan_displays():
    """Return {":N": {"display", "auth", "size", "pid"}} for running Xvfb processes."""
    out = {}
    try:
        text = subprocess.run(["ps", "-eo", "pid=,args="], capture_output=True, text=True, timeout=5).stdout
    except Exception as exc:  # noqa: BLE001
        log(f"ps failed: {exc}")
        return out
    for line in text.splitlines():
        line = line.strip()
        pid, _, args = line.partition(" ")
        m = XVFB_RE.match(os.path.basename(args.split(" ", 1)[0]) + " " + args.split(" ", 1)[-1]) if args else None
        if not m:
            continue
        disp, rest = m.group(1), m.group(2)
        auth = re.search(r"-auth\s+(\S+)", rest)
        size = re.search(r"-screen\s+\d+\s+(\d+)x(\d+)", rest)
        out[disp] = {
            "display": disp,
            "auth": auth.group(1) if auth else None,
            "auth_exists": bool(auth and os.path.exists(auth.group(1))),
            "size": f"{size.group(1)}x{size.group(2)}" if size else "1280x720",
            "pid": int(pid) if pid.isdigit() else None,
        }
    return dict(sorted(out.items(), key=lambda kv: int(kv[0][1:])))


class Capture:
    """One ffmpeg x11grab process; holds only the latest JPEG in memory."""

    def __init__(self, key, info, fps, scale):
        self.key, self.info, self.fps, self.scale = key, info, fps, scale
        self.cond = threading.Condition()
        self.frame, self.seq = None, 0
        self.viewers = 0
        self.idle_since = None
        self.alive = True
        env = dict(os.environ, DISPLAY=info["display"])
        if info.get("auth"):
            env["XAUTHORITY"] = info["auth"]
        cmd = ["ffmpeg", "-nostdin", "-loglevel", "error", "-f", "x11grab", "-draw_mouse", "0",
               "-framerate", str(fps), "-video_size", info["size"], "-i", info["display"]]
        if scale:
            cmd += ["-vf", f"scale={scale}:-2"]
        cmd += ["-f", "mjpeg", "-q:v", "5", "-"]
        self.proc = subprocess.Popen(cmd, env=env, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
                                     stderr=sys.stderr, start_new_session=True)
        log(f"ffmpeg start {key} pid={self.proc.pid}")
        threading.Thread(target=self._reader, daemon=True).start()

    def _reader(self):
        buf = bytearray()
        stream = self.proc.stdout
        while True:
            chunk = stream.read1(65536) if hasattr(stream, "read1") else stream.read(65536)
            if not chunk:
                break
            buf += chunk
            while True:
                start = buf.find(b"\xff\xd8")
                if start < 0:
                    del buf[:-1]
                    break
                end = buf.find(b"\xff\xd9", start + 2)
                if end < 0:
                    if start:
                        del buf[:start]
                    break
                jpeg = bytes(buf[start:end + 2])
                del buf[:end + 2]
                with self.cond:
                    self.frame, self.seq = jpeg, self.seq + 1
                    self.cond.notify_all()
        with self.cond:
            self.alive = False
            self.cond.notify_all()
        log(f"ffmpeg output ended {self.key} rc={self.proc.poll()}")

    def stop(self):
        if self.proc.poll() is None:
            try:
                self.proc.terminate()
                self.proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.proc.kill()
            except Exception:  # noqa: BLE001
                pass
        with self.cond:
            self.alive = False
            self.cond.notify_all()
        log(f"ffmpeg stop {self.key}")


class Registry:
    def __init__(self):
        self.lock = threading.Lock()
        self.caps = {}
        threading.Thread(target=self._reaper, daemon=True).start()

    def acquire(self, info, fps, scale):
        key = (info["display"], fps, scale)
        with self.lock:
            cap = self.caps.get(key)
            if cap is None or not cap.alive or cap.info.get("auth") != info.get("auth"):
                if cap is not None:
                    cap.stop()
                cap = self.caps[key] = Capture(key, info, fps, scale)
            cap.viewers += 1
            cap.idle_since = None
            return cap

    def release(self, cap):
        with self.lock:
            cap.viewers -= 1
            if cap.viewers <= 0:
                cap.idle_since = time.monotonic()

    def _reaper(self):
        while True:
            time.sleep(1)
            stop = []
            with self.lock:
                for key, cap in list(self.caps.items()):
                    idle = cap.viewers <= 0 and cap.idle_since and time.monotonic() - cap.idle_since > IDLE_STOP_SECONDS
                    if idle or (not cap.alive and cap.viewers <= 0):
                        stop.append(self.caps.pop(key))
            for cap in stop:
                cap.stop()

    def stop_all(self):
        with self.lock:
            caps, self.caps = list(self.caps.values()), {}
        for cap in caps:
            cap.stop()

    def status(self):
        with self.lock:
            return [{"display": k[0], "fps": k[1], "scale": k[2], "viewers": c.viewers, "pid": c.proc.pid}
                    for k, c in self.caps.items()]


REGISTRY = Registry()

INDEX_HTML = r"""<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Client live view</title>
<style>
:root{--bg:#111214;--card:#1b1d21;--ink:#e6e6e3;--dim:#8d8f94;--accent:#4c9be8;--bad:#e8774c}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--ink);font:14px/1.4 system-ui,sans-serif}
header{padding:12px 16px;display:flex;gap:12px;align-items:baseline;flex-wrap:wrap}
h1{font-size:17px;margin:0}header span{color:var(--dim)}
#grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(320px,1fr));gap:12px;padding:0 16px 16px}
.card{background:var(--card);border-radius:8px;overflow:hidden;cursor:pointer}
.card img,#detail img{display:block;width:100%;aspect-ratio:16/9;background:#000;object-fit:contain}
.st{padding:6px 10px;color:var(--dim);font:12px/1.4 ui-monospace,monospace;word-break:break-all}
.st b{color:var(--ink)}.bad{color:var(--bad)}
#empty{padding:40px 16px;color:var(--dim)}
#detail{position:fixed;inset:0;background:rgba(0,0,0,.92);display:none;flex-direction:column;justify-content:center;padding:16px}
#detail.open{display:flex}#detail .wrap{max-width:1280px;width:100%;margin:0 auto}
#detail .bar{display:flex;justify-content:space-between;align-items:center;margin-bottom:8px;gap:8px;flex-wrap:wrap}
button{background:var(--card);color:var(--ink);border:1px solid #333;border-radius:6px;padding:4px 10px;cursor:pointer}
</style></head><body>
<header><h1>Client live view</h1><span id="sum">scanning...</span></header>
<div id="grid"></div><div id="empty" hidden>No client running (no Xvfb display found). This page re-scans every 5 s.</div>
<div id="detail"><div class="wrap"><div class="bar"><div class="st" id="dst"></div>
<div><button data-fps="24">24 fps</button> <button data-fps="10">10 fps</button> <button id="close">Close (Esc)</button></div></div>
<img id="dimg" alt=""></div></div>
<script>
const GALLERY_FPS = 1, GALLERY_SCALE = 640;
let displays = [], openDisp = null, detailFps = 24;
const grid = document.getElementById('grid'), detail = document.getElementById('detail'), dimg = document.getElementById('dimg');
const num = d => d.display.slice(1);
function streamUrl(d, fps, scale){ return '/stream/' + num(d) + '?fps=' + fps + (scale ? '&scale=' + scale : '') + '&t=' + Date.now(); }
function statusHtml(d, fps){
  const auth = d.auth ? (d.auth_exists ? d.auth : d.auth + ' <span class="bad">(missing)</span>') : '<span class="bad">no -auth</span>';
  return '<b>' + d.display + '</b> ' + d.size + ' &middot; ' + fps + ' fps &middot; auth ' + auth;
}
function visible(){ return document.visibilityState === 'visible'; }
function setStreams(){
  // Gallery streams only when the tab is visible and no detail view is open; detail only when visible.
  for (const card of grid.children){
    const img = card.querySelector('img'), d = displays.find(x => x.display === card.dataset.disp);
    const want = d && visible() && !openDisp;
    if (want && !img.dataset.on){ img.src = streamUrl(d, GALLERY_FPS, GALLERY_SCALE); img.dataset.on = '1'; }
    if (!want && img.dataset.on){ img.removeAttribute('src'); delete img.dataset.on; }
  }
  const d = openDisp && displays.find(x => x.display === openDisp);
  const key = d ? d.display + '@' + detailFps : '';
  if (d && visible()){
    if (dimg.dataset.on !== key){ dimg.src = streamUrl(d, detailFps, 0); dimg.dataset.on = key; }
    document.getElementById('dst').innerHTML = statusHtml(d, detailFps);
  } else {
    if (dimg.dataset.on){ dimg.removeAttribute('src'); delete dimg.dataset.on; }
    if (openDisp && !d) document.getElementById('dst').innerHTML = '<b>' + openDisp + '</b> <span class="bad">no client running</span>';
  }
}
function render(){
  const keys = displays.map(d => d.display);
  for (const card of [...grid.children]) if (!keys.includes(card.dataset.disp)){ card.querySelector('img').removeAttribute('src'); card.remove(); }
  for (const d of displays){
    let card = grid.querySelector('[data-disp="' + d.display + '"]');
    if (!card){
      card = document.createElement('div'); card.className = 'card'; card.dataset.disp = d.display;
      card.innerHTML = '<img alt=""><div class="st"></div>';
      card.querySelector('img').onerror = () => { const i = card.querySelector('img'); delete i.dataset.on; setTimeout(setStreams, 3000); };
      card.onclick = () => { openDisp = d.display; detail.classList.add('open'); setStreams(); };
      grid.appendChild(card);
    }
    card.querySelector('.st').innerHTML = statusHtml(d, GALLERY_FPS + ' (gallery, ' + GALLERY_SCALE + 'px)');
  }
  document.getElementById('empty').hidden = displays.length > 0;
  document.getElementById('sum').textContent = displays.length + ' display(s) · gallery ' + GALLERY_FPS + ' fps, opened view ' + detailFps + ' fps · streams pause when hidden';
  setStreams();
}
async function scan(){
  try { displays = await (await fetch('/api/displays', {cache:'no-store'})).json(); }
  catch(e){ displays = []; }
  render();
}
function closeDetail(){ openDisp = null; detail.classList.remove('open'); setStreams(); }
document.getElementById('close').onclick = closeDetail;
document.addEventListener('keydown', e => { if (e.key === 'Escape') closeDetail(); });
for (const b of document.querySelectorAll('[data-fps]')) b.onclick = () => { detailFps = +b.dataset.fps; setStreams(); };
dimg.onerror = () => { delete dimg.dataset.on; setTimeout(scan, 2000); };
document.addEventListener('visibilitychange', setStreams);
scan(); setInterval(() => { if (visible()) scan(); }, 5000);
</script></body></html>
"""


class Handler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    server_version = "live_view/1"

    def log_message(self, fmt, *args):
        log(f"{self.client_address[0]} {fmt % args}")

    def _send(self, code, ctype, body):
        self.send_response(code)
        self.send_header("Content-Type", ctype)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        if self.command != "HEAD":
            self.wfile.write(body)

    def do_HEAD(self):
        self.do_GET()

    def do_GET(self):
        url = urlparse(self.path)
        q = parse_qs(url.query)
        if url.path in ("/", "/index.html"):
            return self._send(200, "text/html; charset=utf-8", INDEX_HTML.encode())
        if url.path == "/api/displays":
            return self._send(200, "application/json", json.dumps(list(scan_displays().values())).encode())
        if url.path == "/api/captures":
            return self._send(200, "application/json", json.dumps(REGISTRY.status()).encode())
        m = re.fullmatch(r"/stream/:?(\d+)", url.path)
        if m:
            return self._stream(":" + m.group(1), q)
        return self._send(404, "text/plain", b"not found\n")

    def _stream(self, disp, q):
        try:
            fps = max(1, min(MAX_FPS, int(q.get("fps", ["24"])[0])))
            scale = int(q.get("scale", ["0"])[0])
            scale = max(64, min(3840, scale)) if scale > 0 else 0
        except ValueError:
            return self._send(400, "text/plain", b"bad fps/scale\n")
        info = scan_displays().get(disp)
        if info is None:
            return self._send(404, "text/plain", f"no client running on {disp}\n".encode())
        cap = REGISTRY.acquire(info, fps, scale)
        try:
            self.send_response(200)
            self.send_header("Content-Type", "multipart/x-mixed-replace; boundary=" + BOUNDARY.decode())
            self.send_header("Cache-Control", "no-store")
            self.send_header("Connection", "close")
            self.end_headers()
            self.close_connection = True
            last = 0
            while True:
                with cap.cond:
                    cap.cond.wait_for(lambda: cap.seq != last or not cap.alive, timeout=FRAME_WAIT_SECONDS)
                    if cap.seq == last:
                        break  # ffmpeg ended or stalled (display gone)
                    frame, last = cap.frame, cap.seq
                self.wfile.write(b"--" + BOUNDARY + b"\r\nContent-Type: image/jpeg\r\nContent-Length: "
                                 + str(len(frame)).encode() + b"\r\n\r\n" + frame + b"\r\n")
                self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError, TimeoutError):
            pass
        finally:
            REGISTRY.release(cap)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--host", default="0.0.0.0")
    ap.add_argument("--port", type=int, default=8766, help="default 8766 (8765 serves the progress dashboard)")
    args = ap.parse_args()
    httpd = ThreadingHTTPServer((args.host, args.port), Handler)
    httpd.daemon_threads = True

    def shutdown(*_):
        REGISTRY.stop_all()
        os._exit(0)

    signal.signal(signal.SIGTERM, shutdown)
    signal.signal(signal.SIGINT, shutdown)
    log(f"live_view on http://{args.host}:{args.port}/ displays now: {', '.join(scan_displays()) or 'none'}")
    httpd.serve_forever()


if __name__ == "__main__":
    main()
