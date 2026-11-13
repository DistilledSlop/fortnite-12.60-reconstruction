#!/usr/bin/env python3
"""Live MJPEG preview of the original client's Xvfb display(s) in a browser.

    python3 -I porting/tools/live_view.py [--host 0.0.0.0] [--port 8766]

Pages:
  /                      index: one live view per running Xvfb display (gallery 1 fps, opened view 24 fps)
  /api/displays          JSON list of displays found now
  /stream/<N>?fps=&scale=&cursor=  multipart/x-mixed-replace MJPEG of display :N (default 24 fps, native size)
  POST /api/override/<N>   {"on": true|false}: mark display :N for manual override
  /input/<N>             WebSocket: mouse/keyboard input for display :N, accepted only while it is overridden

Manual override: ticking "Manual override" in the opened view writes the marker
$FN_OVERRIDE_DIR/<N> (default /tmp/fn-manual-override/<N>, content: the Xvfb pid). While it exists the
scripted FN_ACTIONS input steps (client_actions.sh) are skipped and this page forwards the browser's mouse
(absolute, or relative under pointer lock for 3D look) and keyboard to the display through XTEST.

Displays are found on every request from `ps -eo args` (Xvfb :N ... -auth FILE). One ffmpeg x11grab runs per
(display, fps, scale) only while at least one HTTP client watches it, shared between those clients, and is
stopped a few seconds after the last one leaves. No frame is ever written to disk.
"""
import argparse
import base64
import hashlib
import json
import os
import re
import signal
import struct
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
OVERRIDE_DIR = os.environ.get("FN_OVERRIDE_DIR") or "/tmp/fn-manual-override"
WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
# Browser -> server input lines: m X Y (absolute), r DX DY (relative), b BUTTON 0|1, k CODE 0|1, x (release all).
INPUT_RE = re.compile(r"m \d{1,5} \d{1,5}|r -?\d{1,4} -?\d{1,4}|b [1-9] [01]|k [A-Za-z0-9]{1,20} [01]|x")
# KeyboardEvent.code -> X keysym name (physical keys, layout independent); letters/digits/F-keys are derived.
KEYSYMS = {
    "Space": "space", "Enter": "Return", "Escape": "Escape", "Tab": "Tab", "Backspace": "BackSpace",
    "ShiftLeft": "Shift_L", "ShiftRight": "Shift_R", "ControlLeft": "Control_L", "ControlRight": "Control_R",
    "AltLeft": "Alt_L", "AltRight": "Alt_R", "MetaLeft": "Super_L", "MetaRight": "Super_R",
    "CapsLock": "Caps_Lock", "ArrowUp": "Up", "ArrowDown": "Down", "ArrowLeft": "Left", "ArrowRight": "Right",
    "Insert": "Insert", "Delete": "Delete", "Home": "Home", "End": "End", "PageUp": "Prior", "PageDown": "Next",
    "Minus": "minus", "Equal": "equal", "BracketLeft": "bracketleft", "BracketRight": "bracketright",
    "Backslash": "backslash", "Semicolon": "semicolon", "Quote": "apostrophe", "Backquote": "grave",
    "Comma": "comma", "Period": "period", "Slash": "slash", "NumpadEnter": "KP_Enter", "NumpadAdd": "KP_Add",
    "NumpadSubtract": "KP_Subtract", "NumpadMultiply": "KP_Multiply", "NumpadDivide": "KP_Divide",
    "NumpadDecimal": "KP_Decimal",
}


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


def keysym_name(code):
    if re.fullmatch(r"Key[A-Z]", code):
        return code[3].lower()
    if re.fullmatch(r"Digit\d", code):
        return code[5]
    if re.fullmatch(r"F\d{1,2}", code):
        return code
    if re.fullmatch(r"Numpad\d", code):
        return "KP_" + code[6]
    return KEYSYMS.get(code)


def inject_main(disp):
    """Child process (--inject :N): read input lines on stdin and replay them on the display with XTEST.

    A separate process so that Xlib's exit-on-connection-loss cannot take the HTTP server down.
    """
    import ctypes
    from ctypes import c_char_p, c_int, c_ubyte, c_uint, c_ulong, c_void_p
    x11, xtst = ctypes.CDLL("libX11.so.6"), ctypes.CDLL("libXtst.so.6")
    x11.XOpenDisplay.restype, x11.XOpenDisplay.argtypes = c_void_p, [c_char_p]
    x11.XStringToKeysym.restype, x11.XStringToKeysym.argtypes = c_ulong, [c_char_p]
    x11.XKeysymToKeycode.restype, x11.XKeysymToKeycode.argtypes = c_ubyte, [c_void_p, c_ulong]
    x11.XFlush.argtypes = [c_void_p]
    xtst.XTestFakeKeyEvent.argtypes = [c_void_p, c_uint, c_int, c_ulong]
    xtst.XTestFakeButtonEvent.argtypes = [c_void_p, c_uint, c_int, c_ulong]
    xtst.XTestFakeMotionEvent.argtypes = [c_void_p, c_int, c_int, c_int, c_ulong]
    xtst.XTestFakeRelativeMotionEvent.argtypes = [c_void_p, c_int, c_int, c_ulong]
    dpy = x11.XOpenDisplay(disp.encode())
    if not dpy:
        log(f"inject {disp}: cannot open display")
        return 1
    keycodes, keys, buttons = {}, set(), set()

    def release_all():
        for kc in keys:
            xtst.XTestFakeKeyEvent(dpy, kc, 0, 0)
        for b in buttons:
            xtst.XTestFakeButtonEvent(dpy, b, 0, 0)
        keys.clear()
        buttons.clear()

    for raw in iter(sys.stdin.buffer.readline, b""):
        p = raw.decode("ascii", "ignore").split()
        try:
            if not p:
                continue
            if p[0] == "m":
                xtst.XTestFakeMotionEvent(dpy, -1, int(p[1]), int(p[2]), 0)
            elif p[0] == "r":
                xtst.XTestFakeRelativeMotionEvent(dpy, int(p[1]), int(p[2]), 0)
            elif p[0] == "b":
                b, down = int(p[1]), p[2] == "1"
                xtst.XTestFakeButtonEvent(dpy, b, int(down), 0)
                (buttons.add if down else buttons.discard)(b)
            elif p[0] == "k":
                if p[1] not in keycodes:
                    name = keysym_name(p[1])
                    keycodes[p[1]] = x11.XKeysymToKeycode(dpy, x11.XStringToKeysym(name.encode())) if name else 0
                kc, down = keycodes[p[1]], p[2] == "1"
                if kc:
                    xtst.XTestFakeKeyEvent(dpy, kc, int(down), 0)
                    (keys.add if down else keys.discard)(kc)
            elif p[0] == "x":
                release_all()
        except (ValueError, IndexError):
            continue
        x11.XFlush(dpy)
    release_all()
    x11.XFlush(dpy)
    return 0


class Overrides:
    """Displays marked for manual override, their marker files and their input injector processes."""

    def __init__(self):
        self.lock = threading.Lock()
        self.on = {}        # ":N" -> display info (with the Xvfb pid the mark belongs to)
        self.inj = {}       # ":N" -> (Popen, spawn time)

    def _path(self, disp):
        return os.path.join(OVERRIDE_DIR, disp[1:])

    def adopt(self, displays):
        """Startup: keep marks left by a previous live_view for displays that still run, drop the rest."""
        try:
            names = os.listdir(OVERRIDE_DIR)
        except OSError:
            return
        for name in names:
            info = displays.get(":" + name)
            try:
                pid = int(open(os.path.join(OVERRIDE_DIR, name)).read().strip() or 0)
            except (OSError, ValueError):
                pid = 0
            if info and info["pid"] == pid:
                self.on[":" + name] = info
            else:
                self._unlink(":" + name)

    def _unlink(self, disp):
        try:
            os.unlink(self._path(disp))
        except OSError:
            pass

    def _stop_injector(self, disp):
        entry = self.inj.pop(disp, None)
        if entry:
            try:
                entry[0].stdin.close()  # EOF: the child releases held keys/buttons and exits
            except OSError:
                pass

    def reconcile(self, displays):
        """Drop marks whose Xvfb is gone or was replaced (xvfb-run -a reuses display numbers)."""
        with self.lock:
            for disp, info in list(self.on.items()):
                now = displays.get(disp)
                if not now or now["pid"] != info["pid"]:
                    self._clear(disp)

    def _clear(self, disp):
        self.on.pop(disp, None)
        self._stop_injector(disp)
        self._unlink(disp)
        log(f"override off {disp}")

    def set(self, info, on):
        disp = info["display"]
        with self.lock:
            if not on:
                if disp in self.on:
                    self._clear(disp)
                return
            os.makedirs(OVERRIDE_DIR, exist_ok=True)
            tmp = self._path(disp) + ".tmp"
            with open(tmp, "w") as fh:
                fh.write(f"{info['pid']}\n")
            os.replace(tmp, self._path(disp))
            if disp not in self.on:
                log(f"override on {disp} (Xvfb pid {info['pid']})")
            self.on[disp] = info

    def active(self, disp):
        return disp in self.on

    def send(self, disp, line):
        with self.lock:
            info = self.on.get(disp)
            if info is None:
                return
            proc, started = self.inj.get(disp, (None, 0.0))
            if proc is None or proc.poll() is not None:
                if time.monotonic() - started < 1.0:
                    return  # do not respawn a failing injector on every input event
                env = dict(os.environ, DISPLAY=disp)
                if info.get("auth"):
                    env["XAUTHORITY"] = info["auth"]
                proc = subprocess.Popen([sys.executable, "-I", os.path.abspath(__file__), "--inject", disp], env=env,
                                        stdin=subprocess.PIPE, stdout=subprocess.DEVNULL, stderr=sys.stderr,
                                        start_new_session=True)
                self.inj[disp] = (proc, time.monotonic())
                log(f"injector start {disp} pid={proc.pid}")
            try:
                proc.stdin.write(line.encode() + b"\n")
                proc.stdin.flush()
            except OSError:
                pass


OVERRIDES = Overrides()


class Capture:
    """One ffmpeg x11grab process; holds only the latest JPEG in memory."""

    def __init__(self, key, info, fps, scale, cursor=False):
        self.key, self.info, self.fps, self.scale = key, info, fps, scale
        self.cond = threading.Condition()
        self.frame, self.seq = None, 0
        self.viewers = 0
        self.idle_since = None
        self.alive = True
        env = dict(os.environ, DISPLAY=info["display"])
        if info.get("auth"):
            env["XAUTHORITY"] = info["auth"]
        cmd = ["ffmpeg", "-nostdin", "-loglevel", "error", "-f", "x11grab", "-draw_mouse", "1" if cursor else "0",
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

    def acquire(self, info, fps, scale, cursor=False):
        key = (info["display"], fps, scale, cursor)
        with self.lock:
            cap = self.caps.get(key)
            if cap is None or not cap.alive or cap.info.get("auth") != info.get("auth"):
                if cap is not None:
                    cap.stop()
                cap = self.caps[key] = Capture(key, info, fps, scale, cursor)
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
            return [{"display": k[0], "fps": k[1], "scale": k[2], "cursor": k[3], "viewers": c.viewers, "pid": c.proc.pid}
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
label{cursor:pointer;white-space:nowrap}.ctl{display:flex;gap:10px;align-items:center;flex-wrap:wrap}
.ovtag{color:#111;background:var(--bad);border-radius:4px;padding:0 5px;font-weight:600}
#detail.ov #dimg{outline:2px solid var(--bad);cursor:crosshair}
#detail .ovonly{display:none}#detail.ov .ovonly{display:inline-block}
#hint{color:var(--dim);font-size:12px;margin-top:6px;min-height:1.4em}
#detail .wrap:fullscreen{max-width:none;display:flex;flex-direction:column;justify-content:center;background:#000;padding:8px}
#detail .wrap:fullscreen #dimg{max-height:calc(100vh - 80px)}
</style></head><body>
<header><h1>Client live view</h1><span id="sum">scanning...</span></header>
<div id="grid"></div><div id="empty" hidden>No client running (no Xvfb display found). This page re-scans every 5 s.</div>
<div id="detail"><div class="wrap"><div class="bar"><div class="st" id="dst"></div>
<div class="ctl"><label><input type="checkbox" id="ov"> Manual override</label>
<label class="ovonly"><input type="checkbox" id="look" checked> Capture mouse on click</label>
<button class="ovonly" id="esc">Send Esc</button> <button id="full">Fullscreen</button>
<button data-fps="24">24 fps</button> <button data-fps="10">10 fps</button> <button id="close">Close (Esc)</button></div></div>
<img id="dimg" alt="" draggable="false"><div id="hint"></div></div></div>
<script>
const GALLERY_FPS = 1, GALLERY_SCALE = 640;
let displays = [], openDisp = null, detailFps = 24;
const grid = document.getElementById('grid'), detail = document.getElementById('detail'), dimg = document.getElementById('dimg');
const num = d => d.display.slice(1);
function streamUrl(d, fps, scale){ return '/stream/' + num(d) + '?fps=' + fps + (scale ? '&scale=' + scale : '') + '&t=' + Date.now(); }
function statusHtml(d, fps){
  const auth = d.auth ? (d.auth_exists ? d.auth : d.auth + ' <span class="bad">(missing)</span>') : '<span class="bad">no -auth</span>';
  return '<b>' + d.display + '</b> ' + (d.override ? '<span class="ovtag">MANUAL OVERRIDE</span> ' : '') + d.size + ' &middot; ' + fps + ' fps &middot; auth ' + auth;
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
  const key = d ? d.display + '@' + detailFps + (d.override ? 'c' : '') : '';
  if (d && visible()){
    if (dimg.dataset.on !== key){ dimg.src = streamUrl(d, detailFps, 0) + (d.override ? '&cursor=1' : ''); dimg.dataset.on = key; }
    document.getElementById('dst').innerHTML = statusHtml(d, detailFps);
  } else {
    if (dimg.dataset.on){ dimg.removeAttribute('src'); delete dimg.dataset.on; }
    if (openDisp && !d) document.getElementById('dst').innerHTML = '<b>' + openDisp + '</b> <span class="bad">no client running</span>';
  }
  syncInput();
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
function closeDetail(){ if (document.fullscreenElement) document.exitFullscreen(); openDisp = null; detail.classList.remove('open'); setStreams(); }
document.getElementById('close').onclick = closeDetail;
for (const b of document.querySelectorAll('[data-fps]')) b.onclick = () => { detailFps = +b.dataset.fps; setStreams(); };

// ---- manual override: forward mouse and keyboard of the opened display over /input/<N> ----
const ovbox = document.getElementById('ov'), look = document.getElementById('look'), hint = document.getElementById('hint');
let ws = null, wsDisp = null, rdx = 0, rdy = 0, abs = null, raf = 0;
const heldKeys = new Set(), heldBtns = new Set();
const cur = () => openDisp && displays.find(x => x.display === openDisp);
const active = () => !!(ws && wsDisp && wsDisp === openDisp);
const locked = () => document.pointerLockElement === dimg;
function send(line){ if (ws && ws.readyState === 1) ws.send(line); }
function flush(){
  raf = 0;
  if (abs){ send('m ' + abs[0] + ' ' + abs[1]); abs = null; }
  const dx = Math.trunc(rdx), dy = Math.trunc(rdy);
  if (dx || dy){ send('r ' + Math.max(-2000, Math.min(2000, dx)) + ' ' + Math.max(-2000, Math.min(2000, dy))); rdx -= dx; rdy -= dy; }
}
function releaseAll(){ rdx = rdy = 0; abs = null; heldKeys.clear(); heldBtns.clear(); send('x'); }
function setHint(){
  hint.textContent = !active() ? '' : locked() ? 'Mouse captured: movement is relative (3D look). Esc releases the mouse; use "Send Esc" for the game\'s Esc.'
    : 'Manual override: keyboard and mouse go to the client.' + (look.checked ? ' Click the picture to capture the mouse for 3D look.' : ' Mouse is absolute (menus).');
}
function syncInput(){
  const d = cur(), want = d && d.override && visible() ? d.display : null;
  if (wsDisp !== want){
    if (ws){ releaseAll(); ws.close(); }
    ws = null; wsDisp = want;
    if (want){
      const w = ws = new WebSocket((location.protocol === 'https:' ? 'wss://' : 'ws://') + location.host + '/input/' + num(d));
      w.onclose = () => { if (ws === w){ ws = null; wsDisp = null; setHint(); } };  // the next scan reconnects
    } else if (locked()) document.exitPointerLock();
  }
  ovbox.checked = !!(d && d.override); ovbox.disabled = !d;
  detail.classList.toggle('ov', !!want);
  setHint();
}
ovbox.onchange = async () => {
  const d = cur(); if (!d) return;
  ovbox.blur();
  try { await fetch('/api/override/' + num(d), {method:'POST', headers:{'Content-Type':'application/json'}, body: JSON.stringify({on: ovbox.checked})}); } catch(e){}
  scan();
};
look.onchange = () => { look.blur(); if (!look.checked && locked()) document.exitPointerLock(); setHint(); };
function pos(e){
  const r = dimg.getBoundingClientRect(), [W, H] = cur().size.split('x').map(Number);
  const s = Math.min(r.width / W, r.height / H), ox = (r.width - W * s) / 2, oy = (r.height - H * s) / 2;
  const clamp = (v, m) => Math.round(Math.max(0, Math.min(m - 1, v)));
  return [clamp((e.clientX - r.left - ox) / s, W), clamp((e.clientY - r.top - oy) / s, H)];
}
async function lockMouse(){
  try { await dimg.requestPointerLock({unadjustedMovement: true}); }   // raw deltas, no OS acceleration
  catch(e){ try { await dimg.requestPointerLock(); } catch(e2){} }
}
const BTN = {0:1, 1:2, 2:3, 3:8, 4:9};
dimg.addEventListener('mousemove', e => {
  if (!active()) return;
  if (locked()){ rdx += e.movementX; rdy += e.movementY; } else abs = pos(e);
  if (!raf) raf = requestAnimationFrame(flush);
});
dimg.addEventListener('mousedown', e => {
  if (!active()) return;
  e.preventDefault();
  if (look.checked && !locked()){ lockMouse(); return; }   // this click only captures the mouse
  if (!locked()) abs = pos(e);
  flush();
  const b = BTN[e.button]; if (b){ heldBtns.add(b); send('b ' + b + ' 1'); }
});
window.addEventListener('mouseup', e => {
  const b = BTN[e.button]; if (!active() || !heldBtns.has(b)) return;
  flush(); heldBtns.delete(b); send('b ' + b + ' 0');
});
dimg.addEventListener('wheel', e => {
  if (!active()) return;
  e.preventDefault(); flush();
  const b = e.deltaY < 0 ? 4 : 5; send('b ' + b + ' 1\nb ' + b + ' 0');
}, {passive: false});
dimg.addEventListener('contextmenu', e => { if (active()) e.preventDefault(); });
document.addEventListener('keydown', e => {
  if (!active()){ if (e.key === 'Escape') closeDetail(); return; }
  e.preventDefault();
  if (e.repeat || !e.code) return;
  flush(); heldKeys.add(e.code); send('k ' + e.code + ' 1');
}, true);
document.addEventListener('keyup', e => {
  if (!active()) return;
  e.preventDefault();
  if (heldKeys.delete(e.code)) send('k ' + e.code + ' 0');
}, true);
document.getElementById('esc').onclick = e => { e.target.blur(); send('k Escape 1\nk Escape 0'); };
document.getElementById('full').onclick = async e => {
  e.target.blur();
  if (document.fullscreenElement) return document.exitFullscreen();
  try { await dimg.parentElement.requestFullscreen(); if (navigator.keyboard && navigator.keyboard.lock) await navigator.keyboard.lock(); } catch(err){}
};
document.addEventListener('pointerlockchange', () => { if (!locked()){ rdx = rdy = 0; } setHint(); });
window.addEventListener('blur', () => { if (active()) releaseAll(); });
// Ctrl+W and friends cannot be intercepted; at least ask before the tab closes while driving a client.
window.addEventListener('beforeunload', e => { if (active()){ e.preventDefault(); e.returnValue = ''; } });
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
            displays = scan_displays()
            OVERRIDES.reconcile(displays)
            rows = [dict(d, override=OVERRIDES.active(d["display"])) for d in displays.values()]
            return self._send(200, "application/json", json.dumps(rows).encode())
        if url.path == "/api/captures":
            return self._send(200, "application/json", json.dumps(REGISTRY.status()).encode())
        m = re.fullmatch(r"/stream/:?(\d+)", url.path)
        if m:
            return self._stream(":" + m.group(1), q)
        m = re.fullmatch(r"/input/:?(\d+)", url.path)
        if m:
            return self._input(":" + m.group(1))
        return self._send(404, "text/plain", b"not found\n")

    def _same_origin(self):
        """Refuse cross-site pages: a browser-sent Origin must be this server."""
        origin = self.headers.get("Origin")
        return not origin or urlparse(origin).netloc == self.headers.get("Host")

    def do_POST(self):
        m = re.fullmatch(r"/api/override/:?(\d+)", urlparse(self.path).path)
        if not m:
            return self._send(404, "text/plain", b"not found\n")
        if not self._same_origin():
            return self._send(403, "text/plain", b"cross-origin request refused\n")
        try:
            body = json.loads(self.rfile.read(min(int(self.headers.get("Content-Length") or 0), 4096)) or b"{}")
            on = bool(body["on"])
        except (ValueError, KeyError, TypeError):
            return self._send(400, "text/plain", b'expected {"on": true|false}\n')
        disp = ":" + m.group(1)
        displays = scan_displays()
        OVERRIDES.reconcile(displays)
        info = displays.get(disp)
        if info is None:
            return self._send(404, "text/plain", f"no client running on {disp}\n".encode())
        OVERRIDES.set(info, on)
        return self._send(200, "application/json", json.dumps({"display": disp, "override": on}).encode())

    def _ws_read(self):
        head = self.rfile.read(2)
        if len(head) < 2:
            return None, b""
        size = head[1] & 0x7F
        if size == 126:
            size = struct.unpack(">H", self.rfile.read(2))[0]
        elif size == 127:
            size = struct.unpack(">Q", self.rfile.read(8))[0]
        if size > 65536:
            return None, b""
        mask = self.rfile.read(4) if head[1] & 0x80 else None
        data = self.rfile.read(size)
        if len(data) < size:
            return None, b""
        if mask:
            data = bytes(b ^ mask[i & 3] for i, b in enumerate(data))
        return head[0] & 0x0F, data

    def _input(self, disp):
        key = self.headers.get("Sec-WebSocket-Key")
        if self.headers.get("Upgrade", "").lower() != "websocket" or not key:
            return self._send(400, "text/plain", b"websocket endpoint\n")
        if not self._same_origin():
            return self._send(403, "text/plain", b"cross-origin request refused\n")
        self.send_response(101)
        self.send_header("Upgrade", "websocket")
        self.send_header("Connection", "Upgrade")
        self.send_header("Sec-WebSocket-Accept",
                         base64.b64encode(hashlib.sha1((key + WS_GUID).encode()).digest()).decode())
        self.end_headers()
        self.close_connection = True
        try:
            while True:
                op, data = self._ws_read()
                if op is None or op == 8:
                    break
                if op == 9:
                    self.wfile.write(bytes([0x8A, len(data) & 0x7F]) + data[:125])
                    self.wfile.flush()
                elif op == 1:
                    for line in data.decode("ascii", "ignore").split("\n"):
                        if INPUT_RE.fullmatch(line):
                            OVERRIDES.send(disp, line)  # dropped unless the display is overridden
        except (OSError, struct.error):
            pass
        finally:
            OVERRIDES.send(disp, "x")

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
        cap = REGISTRY.acquire(info, fps, scale, q.get("cursor", ["0"])[0] == "1")
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
    ap.add_argument("--inject", metavar=":N", help=argparse.SUPPRESS)  # internal: input injector child
    args = ap.parse_args()
    if args.inject:
        sys.exit(inject_main(args.inject))
    OVERRIDES.adopt(scan_displays())
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
