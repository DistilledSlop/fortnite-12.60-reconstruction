#!/usr/bin/env python3
"""Local HTTP request recorder and fixture responder for protocol discovery."""

import argparse
import fcntl
import hashlib
import json
import math
import os
import re
import socket
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
import qos_echo  # noqa: E402


def quest_event_start(value):
    """Parse a configured calendar tag and Unix or timezone-aware ISO start."""
    tag, separator, raw = value.partition('=')
    if not separator or tag not in {'EventFlag.LobbySeason12', 'EventFlag.LTE_StormTheAgency', 'EventFlag.LTE_Oro'} or not raw:
        raise argparse.ArgumentTypeError('expected supported event tag=ISO_OR_UNIX')
    try:
        try:
            start = float(raw)
        except ValueError:
            start = datetime.fromisoformat(raw.replace('Z', '+00:00'))
            if start.tzinfo is None or start.utcoffset() is None:
                raise ValueError('timezone required')
            start = start.timestamp()
        if not math.isfinite(start):
            raise ValueError('finite timestamp required')
    except (ValueError, OverflowError, OSError) as exc:
        raise argparse.ArgumentTypeError('invalid event start timestamp') from exc
    return tag, start


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


_WALLET_ROUTE = re.compile(r"^/fortnite/api/game/v2/profile/[^/]+/client/(QueryProfile|PurchaseCatalogEntry)/?$", re.IGNORECASE)
_WALLET_CHANGES = {"fullProfileUpdate", "itemAdded", "itemRemoved", "itemQuantityChanged", "itemAttrChanged", "statModified"}
_WALLET_PLATFORMS = {"EpicPC", "PSN", "XboxLive", "Nintendo", "Android", "IOS", "Shared"}


def wallet_trace_entry(method, url, status, response, wallet_ids):
    """Extract only wallet diagnostics; never retain request bodies, paths or headers."""
    match = _WALLET_ROUTE.fullmatch(url.path)
    query = parse_qs(url.query)
    if method != "POST" or not match or query.get("profileId") != ["common_core"]:
        return None
    command = {"queryprofile": "QueryProfile", "purchasecatalogentry": "PurchaseCatalogEntry"}[match.group(1).lower()]
    entry = {"time": datetime.now(timezone.utc).isoformat(), "command": command, "status": status}
    rvn = query.get("rvn", [])
    if len(rvn) == 1 and re.fullmatch(r"-?[0-9]{1,18}", rvn[0]):
        entry["requestRevision"] = int(rvn[0])
    if not isinstance(response, dict):
        return entry
    for key in ("profileRevision", "profileCommandRevision", "profileChangesBaseRevision"):
        value = response.get(key)
        if type(value) is int:
            entry[key] = value

    def currency(item_id, item):
        if not isinstance(item, dict) or str(item.get("templateId", "")).casefold() != "currency:mtxgiveaway":
            return None
        if not isinstance(item_id, str) or not re.fullmatch(r"[a-fA-F0-9-]{16,64}", item_id):
            return None
        wallet_ids.add(item_id)
        result = {"itemId": item_id}
        if type(item.get("quantity")) is int:
            result["quantity"] = item["quantity"]
        attrs = item.get("attributes")
        if isinstance(attrs, dict) and isinstance(attrs.get("platform"), str) and attrs["platform"] in _WALLET_PLATFORMS:
            result["platform"] = attrs["platform"]
        return result

    changes = response.get("profileChanges")
    entries = []
    if isinstance(changes, list):
        for change in changes:
            if not isinstance(change, dict):
                continue
            kind = change.get("changeType")
            if not isinstance(kind, str) or kind not in _WALLET_CHANGES:
                continue
            summary = {"changeType": kind}
            if kind == "fullProfileUpdate":
                profile = change.get("profile")
                items = profile.get("items") if isinstance(profile, dict) else None
                if isinstance(items, dict):
                    summary["currencyItems"] = [value for item_id, item in items.items()
                                                if (value := currency(item_id, item)) is not None]
            elif kind == "itemAdded":
                value = currency(change.get("itemId"), change.get("item"))
                if value is not None:
                    summary["currencyItem"] = value
            elif isinstance(change.get("itemId"), str) and change["itemId"] in wallet_ids and kind in {"itemQuantityChanged", "itemRemoved", "itemAttrChanged"}:
                summary["itemId"] = change["itemId"]
                if kind == "itemQuantityChanged" and type(change.get("quantity")) is int:
                    summary["quantity"] = change["quantity"]
                if kind == "itemAttrChanged" and change.get("attributeName") == "platform" and isinstance(change.get("attributeValue"), str) and change["attributeValue"] in _WALLET_PLATFORMS:
                    summary["platform"] = change["attributeValue"]
            entries.append(summary)
    entry["profileChanges"] = entries
    notifications = response.get("notifications")
    if isinstance(notifications, list):
        rewards = []
        for notification in notifications:
            if not isinstance(notification, dict) or notification.get("type") != "CatalogPurchase":
                continue
            loot = notification.get("lootResult")
            items = loot.get("items") if isinstance(loot, dict) else None
            if not isinstance(items, list):
                continue
            for item in items:
                if not isinstance(item, dict) or str(item.get("itemType", "")).casefold() != "currency:mtxgiveaway":
                    continue
                value = currency(item.get("itemGuid"), {"templateId": item.get("itemType"), "quantity": item.get("quantity"), "attributes": item.get("attributes")})
                if value is not None:
                    rewards.append(value)
        if rewards:
            entry["purchaseRewards"] = rewards
    return entry


def make_handler(routes, record_path, stub=None, wallet_trace_path=None):
    lock = threading.Lock()
    wallet_ids = set()

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
            if wallet_trace_path is not None:
                with lock:
                    entry = wallet_trace_entry(self.command, url, status, response, wallet_ids)
                    if entry is not None:
                        try:
                            fd = os.open(wallet_trace_path, os.O_WRONLY | os.O_CREAT | os.O_APPEND | os.O_NOFOLLOW, 0o600)
                            with os.fdopen(fd, "a", encoding="utf-8") as stream:
                                os.fchmod(stream.fileno(), 0o600)
                                stream.write(json.dumps(entry, sort_keys=True) + "\n")
                        except OSError:
                            pass  # Diagnostic capture must not change the HTTP response.
            self.send_json(status, response)

        def serve_websocket(self, url, serve):
            """Run a websocket route. The socket stays open until serve() returns."""
            self.close_connection = True
            conn = local_ws.accept_upgrade(self, subprotocol=getattr(serve, "subprotocol", None))
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
                "path": api_common.log_safe_path(url.path),
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
    parser.add_argument("--wallet-trace", type=Path, help="opt in to redacted common_core wallet diagnostics (JSONL)")
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
    parser.add_argument("--matchmaking-ws-origin", help="with --stub: advertised ws:// or wss:// origin")
    parser.add_argument("--registered-matchmaking", action="store_true", default=None,
                        help="with --stub: wait for a compatible registered game server (also enabled by FORTNITE_GAMESERVER_SECRET)")
    content_source = parser.add_mutually_exclusive_group()
    content_source.add_argument("--content-dir", type=Path,
                                help="with --stub: lobby content.json and local image directory")
    content_source.add_argument("--content-file", type=Path,
                                help="with --stub: lobby JSON snapshot; assets resolve beside it")
    parser.add_argument("--content-public-origin",
                        help="with --stub: HTTP(S) origin for local lobby image URLs")
    parser.add_argument("--local-matchmaking-ws-port", type=int,
                        help="with --stub: serve an additional plaintext WebSocket listener on loopback (0 chooses a port)")
    parser.add_argument("--local-qos-port", type=int, metavar="PORT",
                        help="with --stub: serve a candidate loopback QoS hotfix and UDP echo (0 chooses a port)")
    parser.add_argument("--quest-event-start", action="append", type=quest_event_start, default=[],
                        metavar="TAG=ISO_OR_UNIX", help="configure a quest calendar event start; repeatable")
    args = parser.parse_args()
    if len(dict(args.quest_event_start)) != len(args.quest_event_start):
        parser.error("duplicate quest event start tag")
    if args.local_matchmaking_ws_port is not None and (not args.stub or args.matchmaking_ws_origin):
        parser.error("--local-matchmaking-ws-port requires --stub and no --matchmaking-ws-origin")
    if args.local_qos_port is not None and (not args.stub or not 0 <= args.local_qos_port <= 65535):
        parser.error("--local-qos-port requires --stub and a port between 0 and 65535")
    args.record.parent.mkdir(parents=True, exist_ok=True)
    routes = load_fixtures(args.fixtures)
    stub = None
    state_lock = None
    if args.stub:
        if not args.ephemeral:
            args.state_dir.mkdir(parents=True, exist_ok=True)
            state_lock = (args.state_dir / ".backend.lock").open("a")
            try:
                fcntl.flock(state_lock, fcntl.LOCK_EX | fcntl.LOCK_NB)
            except BlockingIOError:
                state_lock.close()
                parser.error(f"state directory is already in use: {args.state_dir}")
        import fortnite_stub as stub
        stub.configure(cloudstorage_dir=args.cloudstorage_dir,
                       state_dir=None if args.ephemeral else args.state_dir,
                       matchmaking_ws_origin=args.matchmaking_ws_origin,
                       registered_matchmaking=args.registered_matchmaking,
                       content_dir=args.content_dir, content_file=args.content_file,
                       content_public_origin=args.content_public_origin,
                       quest_event_starts=dict(args.quest_event_start))
    qos_socket = None
    qos_stop = None
    qos_thread = None
    if args.local_qos_port is not None:
        qos_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            qos_socket.bind(("127.0.0.1", args.local_qos_port))
            qos_port = qos_socket.getsockname()[1]
            stub.DEFAULT.cloud.set_system_override("DefaultEngine.ini", qos_echo.hotfix(qos_port))
        except (OSError, ValueError) as exc:
            qos_socket.close()
            parser.error(f"cannot enable local QoS: {exc}")
    server = ThreadingHTTPServer((args.bind, args.port), make_handler(routes, args.record, stub, args.wallet_trace))
    if stub is not None and not args.matchmaking_ws_origin and not args.tls_cert and args.bind == "127.0.0.1":
        # The default local HTTP server must advertise its own reachable socket,
        # including the actual port when the caller requested port zero.
        stub.DEFAULT.matchmaking.websocket_origin = f"ws://127.0.0.1:{server.server_port}"
    ws_server = None
    if args.local_matchmaking_ws_port is not None:
        ws_server = ThreadingHTTPServer(("127.0.0.1", args.local_matchmaking_ws_port),
                                        make_handler(routes, args.record, stub, args.wallet_trace))
        stub.DEFAULT.matchmaking.websocket_origin = f"ws://127.0.0.1:{ws_server.server_port}"
        threading.Thread(target=ws_server.serve_forever, daemon=True).start()
        print(f"Local matchmaking WebSocket on 127.0.0.1:{ws_server.server_port}", flush=True)
    if args.tls_cert:
        import ssl
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain(args.tls_cert, args.tls_key)
        server.socket = context.wrap_socket(server.socket, server_side=True)
    if qos_socket is not None:
        qos_stop = threading.Event()
        qos_thread = threading.Thread(target=qos_echo.serve, args=(qos_socket, qos_stop), daemon=True)
        qos_thread.start()
        print(f"Local QoS echo on 127.0.0.1:{qos_port}", flush=True)
    print(f"Listening on {args.bind}:{server.server_port} with {len(routes)} fixture(s)", flush=True)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    finally:
        server.server_close()
        if qos_thread is not None:
            qos_stop.set()
            qos_thread.join()
            qos_socket.close()
        if ws_server is not None:
            ws_server.shutdown()
            ws_server.server_close()
        if stub is not None:
            stub.DEFAULT.close()
        if state_lock is not None:
            state_lock.close()


if __name__ == "__main__":
    main()
