#!/usr/bin/env python3
"""Smoke test for the lobby-to-match routes of the local stub (stdlib only).

Starts probe_server.py --stub on a local port and checks, over real HTTP and websocket:
ticket -> websocket StatusUpdate states -> Play -> session record -> session key,
plus the negative cases and the ping/pong and close handling.

Plain HTTP and ws:// by default. Pass --tls-cert and --tls-key (for example from
gen_tls_cert.sh) to run the same checks over HTTPS and wss://.

The session key is never printed. Only its decoded length is checked.
Exit status is 0 when every check passes.
"""

import argparse
import base64
import hashlib
import json
import secrets
import socket
import ssl
import struct
import subprocess
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import matchmaking  # noqa: E402

SERVER = HERE / "probe_server.py"
DEFAULT_RECORD = HERE.parents[0] / "generated" / "smoke-matchmaking-requests.jsonl"
ACCOUNT = "00000000000000000000000000000001"
ALL_FIELDS_SESSION = {
    "ownerId", "ownerName", "serverName", "serverAddress", "serverPort", "maxPublicPlayers",
    "openPublicPlayers", "publicPlayers", "privatePlayers", "attributes", "buildUniqueId", "started",
}
STATUS_FIELDS = {"state", "totalPlayers", "connectedPlayers", "queuedPlayers", "estimatedWaitSec",
                 "ticketId", "matchId"}

failures = []


def check(name, ok, detail=""):
    print(("PASS " if ok else "FAIL ") + name + ("" if ok or not detail else f" ({detail})"))
    if not ok:
        failures.append(name)


def free_port():
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        return probe.getsockname()[1]


def http(base, ctx, method, path):
    request = urllib.request.Request(base + path, method=method)
    try:
        with urllib.request.urlopen(request, timeout=10, context=ctx) as response:
            return response.status, response.read()
    except urllib.error.HTTPError as error:
        return error.code, error.read()


def read_exact(stream, count):
    data = stream.read(count)
    if data is None or len(data) < count:
        raise EOFError("connection closed")
    return data


def ws_open(port, path, ctx):
    raw = socket.create_connection(("127.0.0.1", port), timeout=10)
    sock = ctx.wrap_socket(raw, server_hostname="127.0.0.1") if ctx else raw
    key = base64.b64encode(secrets.token_bytes(16)).decode("ascii")
    sock.sendall((
        f"GET {path} HTTP/1.1\r\nHost: 127.0.0.1:{port}\r\nUpgrade: websocket\r\n"
        f"Connection: Upgrade\r\nSec-WebSocket-Key: {key}\r\nSec-WebSocket-Version: 13\r\n\r\n"
    ).encode("ascii"))
    stream = sock.makefile("rb")
    status_line = stream.readline().decode("latin-1").strip()
    headers = {}
    while True:
        line = stream.readline().decode("latin-1").strip()
        if not line:
            break
        name, _, value = line.partition(":")
        headers[name.strip().lower()] = value.strip()
    expected = base64.b64encode(
        hashlib.sha1((key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode("ascii")).digest()
    ).decode("ascii")
    return sock, stream, status_line, headers.get("sec-websocket-accept") == expected


def ws_send(sock, opcode, payload=b""):
    mask = secrets.token_bytes(4)
    frame = bytes([0x80 | opcode, 0x80 | len(payload)]) + mask
    frame += bytes(byte ^ mask[index % 4] for index, byte in enumerate(payload))
    sock.sendall(frame)


def ws_recv(stream):
    """Return (opcode, payload) for one server frame. Server frames are unmasked."""
    first, second = read_exact(stream, 2)
    length = second & 0x7F
    if length == 126:
        length = struct.unpack("!H", read_exact(stream, 2))[0]
    elif length == 127:
        length = struct.unpack("!Q", read_exact(stream, 8))[0]
    return first & 0x0F, read_exact(stream, length)


def run_checks(base, port, ctx):
    status, _ = http(base, ctx, "GET", "/__health")
    check("health endpoint still answers", status == 200, f"status {status}")

    query = urllib.parse.urlencode({"partyPlayerIds": "", "bucketId": "UNKNOWN:0:1",
                                    "player.platform": "Windows", "player.subregions": ""})
    status, raw = http(base, ctx, "GET", f"/fortnite/api/game/v2/matchmakingservice/ticket/player/{ACCOUNT}?{query}")
    ticket = json.loads(raw) if status == 200 else {}
    check("ticket returns 200", status == 200, f"status {status}")
    check("ticket has serviceUrl, ticketType, payload, signature",
          set(ticket) == {"serviceUrl", "ticketType", "payload", "signature"}, str(sorted(ticket)))
    service = urllib.parse.urlsplit(ticket.get("serviceUrl", ""))
    check("serviceUrl is a wss URL on the matchmaking host",
          service.scheme == "wss" and service.hostname == matchmaking.MATCHMAKING_HOST, ticket.get("serviceUrl", ""))
    claims = json.loads(base64.b64decode(ticket.get("payload", "")) or b"{}")
    check("payload decodes and echoes the bucket id", claims.get("bucketId") == "UNKNOWN:0:1", str(sorted(claims)))
    ticket_id = service.path.rsplit("/", 1)[-1]
    matchmaking_ws = service.path

    sock, stream, status_line, accepted = ws_open(port, matchmaking_ws, ctx)
    check("websocket upgrade accepted (101, correct Sec-WebSocket-Accept)",
          status_line.startswith("HTTP/1.1 101") and accepted, status_line)
    messages = []
    try:
        while True:
            opcode, payload = ws_recv(stream)
            if opcode != 0x1:
                break
            messages.append(json.loads(payload))
            if messages[-1].get("name") == "Play":
                break
    except (EOFError, OSError) as error:
        check("websocket delivered messages until Play", False, str(error))

    states = [m.get("state") for m in messages if m.get("name") == "StatusUpdate"]
    check("StatusUpdate states arrive in order",
          states == list(matchmaking.STATES), str(states))
    status_fields_ok = all(set(m) == STATUS_FIELDS | {"name"} for m in messages if m.get("name") == "StatusUpdate")
    check("StatusUpdate messages carry all documented fields", status_fields_ok)
    match_ids = {m.get("matchId") for m in messages if m.get("name") == "StatusUpdate"}
    check("StatusUpdate messages reference one ticket and one match",
          match_ids and len(match_ids) == 1 and all(m.get("ticketId") == ticket_id for m in messages
                                                    if m.get("name") == "StatusUpdate"))
    play = messages[-1] if messages else {}
    check("Play message follows SessionAssignment and carries sessionId and joinDelaySec",
          play.get("name") == "Play" and "sessionId" in play and "joinDelaySec" in play, str(play)[:120])
    session_id = play.get("sessionId", "")

    ws_send(sock, 0x9, b"ping-check")
    opcode, payload = ws_recv(stream)
    check("ping is answered with a pong carrying the same payload", opcode == 0xA and payload == b"ping-check")
    ws_send(sock, 0x8)
    opcode, payload = ws_recv(stream)
    check("client close is echoed with a close frame", opcode == 0x8)
    sock.close()

    sock, stream, status_line, _ = ws_open(port, matchmaking.MATCHMAKING_WS_PATH + "/" + "0" * 32, ctx)
    opcode, payload = ws_recv(stream) if status_line.startswith("HTTP/1.1 101") else (None, b"")
    check("unknown ticket websocket is closed with 1008",
          opcode == 0x8 and struct.unpack("!H", payload[:2])[0] == 1008, f"opcode {opcode}")
    sock.close()

    status, raw = http(base, ctx, "GET", f"/fortnite/api/matchmaking/session/{session_id}")
    record = json.loads(raw) if status == 200 else {}
    check("session record returns 200", status == 200, f"status {status}")
    check("session record has the OnlineSessionMcp fields", set(record) == ALL_FIELDS_SESSION, str(sorted(record)))
    check("session buildUniqueId is 683914758", record.get("buildUniqueId") == "683914758")
    check("session ownerId is the ticket account", record.get("ownerId") == ACCOUNT)
    status, _ = http(base, ctx, "GET", "/fortnite/api/matchmaking/session/" + "0" * 32)
    check("unknown session returns 404", status == 404, f"status {status}")

    key_path = f"/api/game/v2/matchmaking/account/{ACCOUNT}/session/{session_id}"
    status, raw = http(base, ctx, "GET", "/fortnite" + key_path)
    body = json.loads(raw) if status == 200 else {}
    key_value = body.get(matchmaking.SESSION_KEY_FIELD, "")
    check("session key returns 200 with the documented field", status == 200 and bool(key_value),
          f"status {status}")
    try:
        decoded = base64.b64decode(key_value, validate=True)
    except ValueError:
        decoded = b""
    check("session key is base64 of 32 bytes", len(decoded) == matchmaking.SESSION_KEY_BYTES,
          f"decoded length {len(decoded)}")
    status, raw = http(base, ctx, "GET", key_path)
    again = json.loads(raw).get(matchmaking.SESSION_KEY_FIELD) if status == 200 else None
    check("bare-prefix key path also answers, same key for the same session", again == key_value)
    status, _ = http(base, ctx, "GET", f"/fortnite/api/game/v2/matchmaking/account/{ACCOUNT}/session/{'0' * 32}")
    check("key for unknown session returns 404", status == 404, f"status {status}")

    status, raw = http(base, ctx, "GET", "/fortnite/api/storefront/v2/keychain?numKeysDownloaded=0")
    check("existing login route still answers", status == 200 and json.loads(raw), f"status {status}")


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--port", type=int, default=0, help="listen port (default: a free port)")
    parser.add_argument("--tls-cert", type=Path, help="serve HTTPS and wss with this certificate")
    parser.add_argument("--tls-key", type=Path, help="private key for --tls-cert")
    parser.add_argument("--record", type=Path, default=DEFAULT_RECORD,
                        help="request log for the probe (ignored git output under porting/generated)")
    args = parser.parse_args()
    if bool(args.tls_cert) != bool(args.tls_key):
        parser.error("--tls-cert and --tls-key must be given together")

    port = args.port or free_port()
    args.record.parent.mkdir(parents=True, exist_ok=True)
    command = [sys.executable, str(SERVER), "--bind", "127.0.0.1", "--port", str(port), "--stub",
               "--record", str(args.record)]
    scheme = "http"
    ctx = None
    if args.tls_cert:
        command += ["--tls-cert", str(args.tls_cert), "--tls-key", str(args.tls_key)]
        scheme = "https"
        ctx = ssl.create_default_context(cafile=str(args.tls_cert))

    base = f"{scheme}://127.0.0.1:{port}"
    server = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    try:
        deadline = time.time() + 15
        while True:
            try:
                status, _ = http(base, ctx, "GET", "/__health")
                if status == 200:
                    break
            except OSError:
                pass
            if server.poll() is not None or time.time() > deadline:
                raise SystemExit("probe_server did not become ready: " + server.stderr.read().decode()[-500:])
            time.sleep(0.2)
        run_checks(base, port, ctx)
    finally:
        server.terminate()
        try:
            server.wait(timeout=5)
        except subprocess.TimeoutExpired:
            server.kill()

    if failures:
        print(f"{len(failures)} check(s) failed")
        return 1
    print("all checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
