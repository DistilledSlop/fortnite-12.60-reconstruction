"""Local matchmaking ticket, websocket, session and session-key answers.

Labels used in comments and in porting/server/README.md:
  [observed]  name appears in the 12.60 client binary (UTF-16 strings) or in
              porting/original_client_net_protocol.md section 3.
  [guess]     invented for this local stand-in; not verified against the client.

State (tickets, sessions, keys, the ticket signing secret) lives only in memory.
Ids come from the backend's deterministic id source. Keys and the signing secret come
from `secrets`. Nothing is written to disk, and no key is ever logged.
The matchmaking routes take no bearer token, as the earlier smoke test expects.
"""

import base64
import hashlib
import hmac
import json
import re
import secrets
import threading
import time

import local_ws
from api_common import ApiError, Err

# [guess] Host that generated/hosts maps to 127.0.0.1 (listed in gen_hosts.py EXTRA).
MATCHMAKING_HOST = "fortnite-public-service-prod11.ol.epicgames.com"
# [guess] Path prefix of the websocket; the ticket id is appended.
MATCHMAKING_WS_PATH = "/fortnite/api/game/v2/matchmakingservice/ws"
# [given] Value from the task brief, which matches the network checksum in section 3.
# The name buildUniqueId is [observed] in the binary.
BUILD_UNIQUE_ID = "683914758"
# [guess] Dedicated server address and ports. smoke_join.sh serves UDP 7777 in the same namespace.
SESSION_SERVER_ADDRESS = "127.0.0.1"
SESSION_SERVER_PORT = 7777
SESSION_BEACON_PORT = 7778
# [guess] Seconds the client should wait before joining after the Play message.
JOIN_DELAY_SEC = 0
# [guess] Pause between StatusUpdate messages, so the client can show each state.
STATE_STEP_SEC = 0.5
# [guess] Idle time before the server closes a websocket that has finished matchmaking.
IDLE_TIMEOUT_SEC = 60

# [observed] Status states named in the binary and in section 3. Connecting is first.
STATES = ("Connecting", "Waiting", "Queued", "ReadyCheck", "SessionAssignment")
# [observed] Message names, sent as the type of each frame.
STATUS_UPDATE_MESSAGE = "StatusUpdate"
PLAY_MESSAGE = "Play"
# [guess] Envelope: the message name is a top-level field called "name" and the rest of the
# fields are flat next to it. Change MESSAGE_NAME_FIELD if the client expects another shape.
MESSAGE_NAME_FIELD = "name"
# [guess] Ticket type string returned by the ticket endpoint.
TICKET_TYPE = "mms-player"
# The client response reader names accountId, sessionId and key; see EVIDENCE.md.
SESSION_KEY_FIELD = "key"
SESSION_KEY_BYTES = 32

_TICKET_WS_RE = re.compile(re.escape(MATCHMAKING_WS_PATH) + r"/([0-9a-f]{32})")


def _message(name, **fields):
    return {MESSAGE_NAME_FIELD: name, **fields}


class MatchmakingService:
    """Ticket, websocket, session and key state for one backend instance."""

    def __init__(self, ctx, accounts, state_step_sec=STATE_STEP_SEC):
        self.ctx = ctx
        self.accounts = accounts
        self.state_step_sec = state_step_sec
        self._lock = threading.Lock()
        # [guess] Per-process secret used to HMAC the ticket payload. Never persisted.
        self._ticket_signing_key = secrets.token_bytes(32)
        self._tickets = {}
        self._sessions = {}
        self._session_keys = {}

    def _session_record(self, owner_id, owner_name):
        return {
            "ownerId": owner_id,  # [guess] field value; name [observed] in binary
            "ownerName": owner_name,  # [guess]
            "serverName": "local-dedicated",  # [guess]
            "serverAddress": SESSION_SERVER_ADDRESS,  # [guess]
            "serverPort": SESSION_SERVER_PORT,  # [guess]
            "maxPublicPlayers": 100,  # [guess]
            "openPublicPlayers": 99,  # [guess] max minus the owner
            "publicPlayers": [owner_id],  # [guess]
            "privatePlayers": [],  # [guess]
            "attributes": {"BeaconPort": SESSION_BEACON_PORT},  # [observed] name "BeaconPort"; value [guess]
            "buildUniqueId": BUILD_UNIQUE_ID,  # [observed] name; value [given]
            "started": False,  # [guess]
        }

    def ticket(self, account_id, query):
        """Answer GET .../matchmakingservice/ticket/player/{id}. Returns (status, body)."""
        ticket_id = self.ctx.new_id("ticket")
        match_id = self.ctx.new_id("match")
        session_id = self.ctx.new_id("session")
        claims = {  # [guess] contents of the opaque payload
            "ticketId": ticket_id,
            "matchId": match_id,
            "sessionId": session_id,
            "playerId": account_id,
            "bucketId": query.get("bucketId", ""),
            "partyPlayerIds": query.get("partyPlayerIds", ""),
            "platform": query.get("player.platform", ""),
            "subregions": query.get("player.subregions", ""),
        }
        payload = base64.b64encode(json.dumps(claims, sort_keys=True).encode("utf-8")).decode("ascii")
        signature = base64.b64encode(
            hmac.new(self._ticket_signing_key, payload.encode("ascii"), hashlib.sha256).digest()
        ).decode("ascii")
        owner_name = self.accounts.display_name(account_id)
        with self._lock:
            self._tickets[ticket_id] = {"matchId": match_id, "sessionId": session_id}
            self._sessions[session_id] = self._session_record(account_id, owner_name)
        body = {  # [observed] field names per section 3; values [guess]
            "serviceUrl": f"wss://{MATCHMAKING_HOST}{MATCHMAKING_WS_PATH}/{ticket_id}",
            "ticketType": TICKET_TYPE,
            "payload": payload,
            "signature": signature,
        }
        return 200, body

    def websocket_route(self, path, query):
        """Return a serve(conn) callable for a matchmaking websocket path, or None."""
        match = _TICKET_WS_RE.fullmatch(path)
        if match is None:
            return None
        ticket_id = match.group(1)
        return lambda conn: self._serve_ticket(ticket_id, conn)

    def _serve_ticket(self, ticket_id, conn):
        with self._lock:
            entry = self._tickets.get(ticket_id)
        if entry is None:
            conn.close(1008, "unknown ticket")
            return
        for state in STATES:
            queued = 1 if state in ("Queued", "ReadyCheck", "SessionAssignment") else 0
            conn.send_json(_message(
                STATUS_UPDATE_MESSAGE,
                state=state,  # [observed] state names
                totalPlayers=1,  # [observed] field name; value [guess]
                connectedPlayers=1,  # [observed] field name; value [guess]
                queuedPlayers=queued,  # [observed] field name; value [guess]
                estimatedWaitSec=0,  # [observed] field name; value [guess]
                ticketId=ticket_id,  # [observed] field name
                matchId=entry["matchId"],  # [observed] field name
            ))
            if self.state_step_sec:
                time.sleep(self.state_step_sec)
        conn.send_json(_message(
            PLAY_MESSAGE,
            sessionId=entry["sessionId"],  # [observed] field name
            joinDelaySec=JOIN_DELAY_SEC,  # [observed] field name; value [guess]
        ))
        # Keep the socket open until the client closes it. Frames the client sends are read
        # and discarded, and pings are answered. Idle sockets time out.
        try:
            conn.sock.settimeout(IDLE_TIMEOUT_SEC)
            while True:
                conn.recv_message()
        except (local_ws.WebSocketClosed, OSError):
            pass
        conn.close()

    def session(self, session_id):
        """Answer GET .../matchmaking/session/{id}. Returns (status, body) or None if unknown."""
        with self._lock:
            record = self._sessions.get(session_id)
            if record is None:
                return None
            return 200, dict(record, publicPlayers=list(record["publicPlayers"]),
                             privatePlayers=list(record["privatePlayers"]),
                             attributes=dict(record["attributes"]))

    def session_key(self, account_id, session_id):
        """Answer GET .../matchmaking/account/{acct}/session/{sid}. None if the session is unknown.

        The key is 32 random bytes, created on first request for the session and kept in
        memory only. The account id is not checked, because this is a local stand-in.
        """
        with self._lock:
            if session_id not in self._sessions:
                return None
            key = self._session_keys.get(session_id)
            if key is None:
                key = secrets.token_bytes(SESSION_KEY_BYTES)
                self._session_keys[session_id] = key
        return 200, {"accountId": account_id, "sessionId": session_id,
                     SESSION_KEY_FIELD: base64.b64encode(key).decode("ascii")}

    def register(self, router):
        router.add("GET", r"(?:/fortnite)?/api/game/v2/matchmakingservice/ticket/player/([^/]+)",
                   lambda req, match: self.ticket(match.group(1), req.query),
                   name="matchmaking.ticket", status="observed")
        router.add("GET", r"(?:/fortnite)?/api/matchmaking/session/([^/]+)",
                   lambda req, match: _found(self.session(match.group(1)), match.group(0)),
                   name="matchmaking.session", status="observed")
        router.add("GET", r"(?:/fortnite)?/api/game/v2/matchmaking/account/([^/]+)/session/([^/]+)",
                   lambda req, match: _found(self.session_key(match.group(1), match.group(2)), match.group(0)),
                   name="matchmaking.session_key", status="observed")


def _found(result, path):
    """Unwrap a (status, body) pair, or answer 404 when the lookup returned None."""
    if result is None:
        raise ApiError(404, Err.NOT_FOUND, f"Sorry, the resource {path} was not found", numeric=1004)
    return result
