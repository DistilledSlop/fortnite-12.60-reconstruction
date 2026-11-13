"""The backend and native dedicated server can share their configured network key."""

# Isolated mode omits the script directory from sys.path.
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import base64
import json
import os
import tempfile
import unittest
from unittest.mock import patch
from urllib.parse import urlsplit

from backend import Backend


class MatchmakingNetworkKeyTests(unittest.TestCase):
    def session_key(self):
        backend = Backend(state_step_sec=0)
        account = "a" * 32
        _, ticket = backend.matchmaking.ticket(account, {})
        # The ticket payload identifies the session which the Play message will return.
        session = json.loads(base64.b64decode(ticket["payload"]))["sessionId"]
        status, response = backend.matchmaking.session_key(account, session)
        self.assertEqual(status, 200)
        return base64.b64decode(response["key"])

    def test_environment_hex_and_file_base64_match_server_key(self):
        key = bytes(range(32))
        with tempfile.NamedTemporaryFile(mode="w", encoding="utf-8") as file:
            file.write(base64.b64encode(key).decode() + "\n")
            file.flush()
            for env in ({"FORTNITE_NET_ENCRYPTION_KEY": key.hex()},
                        {"FORTNITE_NET_ENCRYPTION_KEY_FILE": file.name},
                        {"FORTNITE_NET_ENCRYPTION_KEY": key.hex(),
                         "FORTNITE_NET_ENCRYPTION_KEY_FILE": "/nonexistent"}):
                with self.subTest(source=sorted(env)), patch.dict(os.environ, env, clear=True):
                    self.assertEqual(self.session_key(), key)

    def test_independent_players_receive_same_local_server_and_configured_key(self):
        class Connection:
            # Exercise the ticket's emitted Play and session lookups without sockets.
            def __init__(self):
                self.messages = []
                self.sock = self

            def send_json(self, message):
                self.messages.append(message)

            def settimeout(self, seconds):
                pass

            def recv_message(self):
                raise OSError("client closed")

            def close(self):
                pass

        key = bytes(range(32))
        with patch.dict(os.environ, {"FORTNITE_NET_ENCRYPTION_KEY": key.hex()}, clear=True):
            backend = Backend(state_step_sec=0)
            try:
                endpoints = []
                for account in ("a" * 32, "b" * 32):
                    _, ticket = backend.matchmaking.ticket(account, {})
                    conn = Connection()
                    # Use the advertised URL, just as an independent client does.
                    backend.matchmaking.websocket_route(urlsplit(ticket["serviceUrl"]).path, {})(conn)
                    play = next(m["payload"] for m in conn.messages if m["name"] == "Play")
                    status, session = backend.matchmaking.session(play["sessionId"])
                    self.assertEqual(status, 200)
                    endpoints.append((session["serverAddress"], session["serverPort"]))
                    self.assertEqual(backend.matchmaking.join(play["sessionId"], account), (204, None))
                    status, response = backend.matchmaking.session_key(account, play["sessionId"])
                    self.assertEqual(status, 200)
                    self.assertEqual(base64.b64decode(response["key"]), key)
                self.assertEqual(endpoints, [("127.0.0.1", 7777)] * 2)
            finally:
                backend.close()

    def test_invalid_configuration_fails_without_disclosing_value(self):
        with patch.dict(os.environ, {"FORTNITE_NET_ENCRYPTION_KEY": "invalid-secret"}, clear=True):
            with self.assertRaises(ValueError) as error:
                Backend()
            self.assertNotIn("invalid-secret", str(error.exception))

    def test_absent_configuration_keeps_per_session_keys(self):
        with patch.dict(os.environ, {}, clear=True):
            self.assertNotEqual(self.session_key(), self.session_key())


if __name__ == "__main__":
    unittest.main()
