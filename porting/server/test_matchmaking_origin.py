"""A locally advertised matchmaking URL is usable without Epic DNS overrides."""
import json
import socket
import subprocess
import sys
import tempfile
import unittest
import urllib.parse
from pathlib import Path

import smoke_matchmaking as ws
from backend import Backend
from test_services import LOCAL, jcall


class MatchmakingOriginTests(unittest.TestCase):
    def test_explicit_local_origin_points_to_the_running_websocket(self):
        with socket.socket() as reservation:
            reservation.bind(("127.0.0.1", 0))
            port = reservation.getsockname()[1]
        origin = f"ws://127.0.0.1:{port}"
        with tempfile.TemporaryDirectory() as directory:
            command = [sys.executable, "-I", str(Path(__file__).with_name("probe_server.py")),
                       "--stub", "--ephemeral", "--port", str(port), "--matchmaking-ws-origin", origin,
                       "--record", str(Path(directory) / "requests")]
            process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            try:
                self.assertIn("Listening", process.stdout.readline())
                status, ticket, _ = jcall(f"http://127.0.0.1:{port}", "GET",
                                          f"/fortnite/api/game/v2/matchmakingservice/ticket/player/{LOCAL}")
                self.assertEqual(status, 200)
                url = urllib.parse.urlsplit(ticket["serviceUrl"])
                self.assertEqual((url.scheme, url.hostname, url.port), ("ws", "127.0.0.1", port))
                sock, stream, status_line, accepted = ws.ws_open(url.port, url.path, None)
                try:
                    self.assertTrue(status_line.startswith("HTTP/1.1 101") and accepted)
                    opcode, payload = ws.ws_recv(stream)
                    self.assertEqual((opcode, json.loads(payload)["name"]), (1, "StatusUpdate"))
                finally:
                    sock.close()
            finally:
                process.terminate()
                process.communicate(timeout=10)

    def test_invalid_origin_is_rejected(self):
        for origin in ("https://example.com", "ws://example.com/path", "ws://user@example.com", "ws://host:99999"):
            with self.subTest(origin=origin), self.assertRaises(ValueError):
                Backend(matchmaking_ws_origin=origin)


if __name__ == "__main__":
    unittest.main()
