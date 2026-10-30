#!/usr/bin/env python3
"""Local HTTP request recorder and fixture responder for protocol discovery."""

import argparse
import hashlib
import json
import re
import threading
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from string import Template
from urllib.parse import parse_qsl, urlsplit


def compile_route(path):
    parts = path.strip("/").split("/") if path != "/" else []
    pattern = []
    for part in parts:
        if re.fullmatch(r"\{[A-Za-z_][A-Za-z_0-9]*\}", part):
            pattern.append(f"(?P<{part[1:-1]}>[^/]+)")
        else:
            pattern.append(re.escape(part))
    return re.compile("^/" + "/".join(pattern) + "/?$", re.IGNORECASE)


def substitute(value, variables):
    if isinstance(value, str):
        return Template(value).safe_substitute(variables)
    if isinstance(value, list):
        return [substitute(item, variables) for item in value]
    if isinstance(value, dict):
        return {key: substitute(item, variables) for key, item in value.items()}
    return value


def load_fixtures(path):
    if path is None:
        return []
    data = json.loads(path.read_text())
    if not isinstance(data, list):
        raise ValueError("Fixtures must be a JSON array")
    routes = []
    for item in data:
        if not isinstance(item, dict) or not {"method", "path", "status", "body"} <= item.keys():
            raise ValueError("Each fixture needs method, path, status, and body")
        status = int(item["status"])
        if not 100 <= status <= 599:
            raise ValueError("Fixture status must be an HTTP status code")
        routes.append((item["method"].upper(), compile_route(item["path"]), status, item["body"]))
    return routes


def make_handler(routes, record_path):
    lock = threading.Lock()

    class Handler(BaseHTTPRequestHandler):
        protocol_version = "HTTP/1.1"

        def do_GET(self):
            self.handle_request()

        def do_POST(self):
            self.handle_request()

        def do_PUT(self):
            self.handle_request()

        def do_PATCH(self):
            self.handle_request()

        def do_DELETE(self):
            self.handle_request()

        def handle_request(self):
            url = urlsplit(self.path)
            length = int(self.headers.get("Content-Length", "0"))
            if length < 0 or length > 1024 * 1024:
                self.send_json(413, {"error": "request body exceeds 1 MiB"})
                return
            body = self.rfile.read(length)
            if url.path == "/__health":
                status, response = 200, {"status": "ready"}
            else:
                status, response = 501, {"error": "no fixture for this request"}
                for method, pattern, fixture_status, fixture_body in routes:
                    match = pattern.fullmatch(url.path)
                    if method == self.command and match:
                        status = fixture_status
                        response = substitute(fixture_body, match.groupdict())
                        break

            entry = {
                "time": datetime.now(timezone.utc).isoformat(),
                "method": self.command,
                "path": url.path,
                "query_keys": sorted({key for key, _ in parse_qsl(url.query)}),
                "content_type": self.headers.get("Content-Type", ""),
                "body_bytes": len(body),
                "body_sha256": hashlib.sha256(body).hexdigest() if body else None,
                "status": status,
            }
            with lock:
                with record_path.open("a", encoding="utf-8") as stream:
                    stream.write(json.dumps(entry, sort_keys=True) + "\n")
            self.send_json(status, response)

        def send_json(self, status, response):
            data = json.dumps(response, separators=(",", ":")).encode("utf-8")
            self.send_response(status)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(data)))
            self.end_headers()
            self.wfile.write(data)

        def log_message(self, format_string, *args):
            return

    return Handler


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bind", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8080)
    parser.add_argument("--fixtures", type=Path)
    parser.add_argument("--record", type=Path, default=Path(__file__).resolve().parents[1] / "generated/server-requests.jsonl")
    args = parser.parse_args()
    args.record.parent.mkdir(parents=True, exist_ok=True)
    routes = load_fixtures(args.fixtures)
    server = ThreadingHTTPServer((args.bind, args.port), make_handler(routes, args.record))
    print(f"Listening on {args.bind}:{server.server_port} with {len(routes)} fixture(s)", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
