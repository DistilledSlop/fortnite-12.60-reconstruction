"""Minimal RFC 6455 server side for the local stand-in backend (Python stdlib only).

Handles the opening handshake on a BaseHTTPRequestHandler, text/binary messages
with fragmentation, ping/pong and close. Not a general WebSocket library: no
extensions (no permessage-deflate).
"""

import base64
import hashlib
import json
import struct
import threading

WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
MAX_MESSAGE_BYTES = 256 * 1024

OP_CONT = 0x0
OP_TEXT = 0x1
OP_BINARY = 0x2
OP_CLOSE = 0x8
OP_PING = 0x9
OP_PONG = 0xA


class WebSocketClosed(Exception):
    """The peer closed the socket or sent a frame this server cannot accept."""


def accept_upgrade(handler, subprotocol=None):
    """Answer a WebSocket upgrade on handler. Returns a WebSocket, or None after a 400."""
    offered = [item.strip() for item in handler.headers.get("Sec-WebSocket-Protocol", "").split(",")]
    if subprotocol is not None and subprotocol not in offered:
        handler.send_response_only(400)
        handler.send_header("Content-Length", "0")
        handler.end_headers()
        return None
    key = handler.headers.get("Sec-WebSocket-Key", "")
    if not key:
        body = b"missing Sec-WebSocket-Key"
        handler.send_response_only(400)
        handler.send_header("Content-Type", "text/plain")
        handler.send_header("Content-Length", str(len(body)))
        handler.end_headers()
        handler.wfile.write(body)
        return None
    accept = base64.b64encode(hashlib.sha1((key + WS_GUID).encode("ascii")).digest()).decode("ascii")
    handler.send_response_only(101, "Switching Protocols")
    handler.send_header("Upgrade", "websocket")
    handler.send_header("Connection", "Upgrade")
    handler.send_header("Sec-WebSocket-Accept", accept)
    if subprotocol is not None:
        handler.send_header("Sec-WebSocket-Protocol", subprotocol)
    handler.end_headers()
    handler.wfile.flush()
    return WebSocket(handler.rfile, handler.wfile, handler.connection)


class WebSocket:
    def __init__(self, rfile, wfile, sock):
        self.rfile = rfile
        self.wfile = wfile
        self.sock = sock
        self.closed = False
        self._write_lock = threading.RLock()

    def _send_frame(self, opcode, payload=b""):
        header = bytearray([0x80 | opcode])
        size = len(payload)
        if size < 126:
            header.append(size)
        elif size < 65536:
            header.append(126)
            header += struct.pack("!H", size)
        else:
            header.append(127)
            header += struct.pack("!Q", size)
        with self._write_lock:
            self.wfile.write(bytes(header) + payload)
            self.wfile.flush()

    def send_text(self, text):
        self._send_frame(OP_TEXT, text.encode("utf-8"))

    def send_json(self, obj):
        self.send_text(json.dumps(obj, separators=(",", ":")))

    def close(self, code=1000, reason=""):
        if self.closed:
            return
        self.closed = True
        try:
            self._send_frame(OP_CLOSE, struct.pack("!H", code) + reason.encode("utf-8")[:120])
        except OSError:
            pass

    def _read_exact(self, count):
        data = self.rfile.read(count) if count else b""
        if len(data) < count:
            raise WebSocketClosed("peer closed the connection")
        return data

    def _read_frame(self):
        first, second = self._read_exact(2)
        fin = bool(first & 0x80)
        opcode = first & 0x0F
        masked = bool(second & 0x80)
        length = second & 0x7F
        if length == 126:
            length = struct.unpack("!H", self._read_exact(2))[0]
        elif length == 127:
            length = struct.unpack("!Q", self._read_exact(8))[0]
        if length > MAX_MESSAGE_BYTES:
            raise WebSocketClosed("frame exceeds MAX_MESSAGE_BYTES")
        mask = self._read_exact(4) if masked else b""
        payload = self._read_exact(length)
        if masked:
            payload = bytes(byte ^ mask[index % 4] for index, byte in enumerate(payload))
        return fin, opcode, payload

    def recv_message(self):
        """Return (opcode, payload) for the next data message.

        Pings are answered and pongs ignored. A close frame is echoed and raises
        WebSocketClosed, as does any socket error or timeout the caller sees.
        """
        chunks = []
        message_opcode = None
        total = 0
        while True:
            fin, opcode, payload = self._read_frame()
            if opcode == OP_PING:
                self._send_frame(OP_PONG, payload)
                continue
            if opcode == OP_PONG:
                continue
            if opcode == OP_CLOSE:
                self.close(1000)
                raise WebSocketClosed("client sent close")
            if opcode in (OP_TEXT, OP_BINARY):
                if message_opcode is not None:
                    raise WebSocketClosed("new data frame inside a fragmented message")
                message_opcode = opcode
            elif opcode != OP_CONT or message_opcode is None:
                raise WebSocketClosed(f"unexpected opcode {opcode:#x}")
            total += len(payload)
            if total > MAX_MESSAGE_BYTES:
                raise WebSocketClosed("message exceeds MAX_MESSAGE_BYTES")
            chunks.append(payload)
            if fin:
                return message_opcode, b"".join(chunks)
