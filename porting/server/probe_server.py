#!/usr/bin/env python3
"""Local HTTP request recorder and fixture responder for protocol discovery."""

import argparse
import hashlib
import json
import os
import re
import sys
import threading
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from string import Template
from urllib.parse import parse_qs, parse_qsl, urlsplit

# Run with `python3 -I`, which leaves the script directory off sys.path. Add it explicitly.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import api_common  # noqa: E402
import local_ws  # noqa: E402


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


def make_handler(routes, record_path, stub=None):
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
            if stub is not None and self.headers.get("Upgrade", "").lower() == "websocket":
                serve = stub.websocket_route(url.path, dict(parse_qsl(url.query)))
                if serve is not None:
                    self.serve_websocket(url, serve)
                    return
            length = int(self.headers.get("Content-Length", "0"))
            if length < 0 or length > 1024 * 1024:
                self.send_json(413, {"error": "request body exceeds 1 MiB"})
                return
            body = self.rfile.read(length)
            form = {}
            if url.path == "/__health":
                status, response = 200, {"status": "ready"}
            else:
                status, response = 501, {"error": "no fixture for this request"}
                if "x-www-form-urlencoded" in self.headers.get("Content-Type", ""):
                    form = dict(parse_qsl(body.decode("utf-8", "replace")))
                if stub is not None:
                    status, response = stub.respond(
                        self.command, url.path, query=dict(parse_qsl(url.query)), form=form, body=body,
                        headers={key.lower(): value for key, value in self.headers.items()},
                        query_all=parse_qs(url.query))
                for method, pattern, fixture_status, fixture_body in routes:
                    match = pattern.fullmatch(url.path)
                    if method == self.command and match:
                        status = fixture_status
                        response = substitute(fixture_body, match.groupdict())
                        break

            self.record_request(url, body, form, status)
            self.send_json(status, response)

        def serve_websocket(self, url, serve):
            """Run a websocket route. The socket stays open until serve() returns."""
            self.close_connection = True
            conn = local_ws.accept_upgrade(self)
            if conn is None:
                self.record_request(url, b"", {}, 400)
                return
            try:
                serve(conn)
            except (local_ws.WebSocketClosed, OSError):
                pass
            finally:
                conn.close()
            self.record_request(url, b"", {}, 101)

        def record_request(self, url, body, form, status):
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
            query = dict(parse_qsl(url.query))
            # Only non-secret selector values are recorded.
            safe = {key: query[key] for key in ("profileId", "rvn", "bucketId") if key in query}
            if safe:
                entry["query_values"] = safe
            if form:
                # Names only, plus the non-secret grant type; never values such as passwords or codes.
                entry["form_keys"] = sorted(form)
                if "grant_type" in form:
                    entry["grant_type"] = form["grant_type"]
            with lock:
                with record_path.open("a", encoding="utf-8") as stream:
                    stream.write(json.dumps(entry, sort_keys=True) + "\n")

        def send_json(self, status, response):
            content_type = "application/json"
            if isinstance(response, api_common.Raw):
                data, content_type = response.data, response.content_type
            elif isinstance(response, api_common.RawText):
                data, content_type = response.encode("utf-8"), "text/plain"
            else:
                data = b"" if response is None else json.dumps(response, separators=(",", ":")).encode("utf-8")
            self.send_response(status)
            self.send_header("Content-Type", content_type)
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
    parser.add_argument("--stub", action="store_true", help="answer known Epic endpoints with minimal responses")
    parser.add_argument("--tls-cert", type=Path)
    parser.add_argument("--tls-key", type=Path)
    parser.add_argument("--cloudstorage-dir", type=Path,
                        help="with --stub: directory whose files are listed as cloud storage system files")
    parser.add_argument("--state-dir", type=Path,
                        default=Path(__file__).resolve().parents[1] / "generated/local-backend-state",
                        help="with --stub: persistent local account, profile and user-file directory")
    parser.add_argument("--ephemeral", action="store_true",
                        help="with --stub: keep account, profile and user-file state in memory")
    args = parser.parse_args()
    args.record.parent.mkdir(parents=True, exist_ok=True)
    routes = load_fixtures(args.fixtures)
    stub = None
    if args.stub:
        import fortnite_stub as stub
        stub.configure(cloudstorage_dir=args.cloudstorage_dir,
                       state_dir=None if args.ephemeral else args.state_dir)
    server = ThreadingHTTPServer((args.bind, args.port), make_handler(routes, args.record, stub))
    if args.tls_cert:
        import ssl
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain(args.tls_cert, args.tls_key)
        server.socket = context.wrap_socket(server.socket, server_side=True)
    print(f"Listening on {args.bind}:{server.server_port} with {len(routes)} fixture(s)", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
