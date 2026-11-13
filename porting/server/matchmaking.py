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
import binascii
import hashlib
import hmac
import json
import os
import re
import secrets
import threading
import time
from urllib.parse import urlsplit

import local_ws
from api_common import ApiError, Err

# [guess] Host that generated/hosts maps to 127.0.0.1 (listed in gen_hosts.py EXTRA).
MATCHMAKING_HOST = "fortnite-public-service-prod11.ol.epicgames.com"
# [guess] Path prefix of the websocket; the ticket id is appended.
MATCHMAKING_WS_PATH = "/fortnite/api/game/v2/matchmakingservice/ws"
# [observed] The client build-ID reader falls back to its network compatible
# changelist (0xc4c8e5), not its network version checksum.
BUILD_UNIQUE_ID = "12896485"
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
# The client reads a top-level name and nested payload object (see EVIDENCE.md).
MESSAGE_NAME_FIELD = "name"
# [guess] Ticket type string returned by the ticket endpoint.
TICKET_TYPE = "mms-player"
# The client response reader names accountId, sessionId and key; see EVIDENCE.md.
SESSION_KEY_FIELD = "key"
SESSION_KEY_BYTES = 32

_TICKET_WS_RE = re.compile(re.escape(MATCHMAKING_WS_PATH) + r"/([0-9a-f]{32})")


def _message(name, **fields):
    return {MESSAGE_NAME_FIELD: name, "payload": fields}


def _configured_network_key():
    """Read the same optional key configuration as the native dedicated server."""
    value = os.environ.get("FORTNITE_NET_ENCRYPTION_KEY", "")
    if not value:
        path = os.environ.get("FORTNITE_NET_ENCRYPTION_KEY_FILE", "")
        if not path:
            return None
        try:
            with open(path, encoding="utf-8") as key_file:
                value = key_file.read()
        except (OSError, UnicodeError):
            raise ValueError("Cannot read FORTNITE_NET_ENCRYPTION_KEY_FILE") from None
    value = value.strip()
    try:
        if len(value) == SESSION_KEY_BYTES * 2 and re.fullmatch(r"[0-9a-fA-F]+", value):
            key = bytes.fromhex(value)
        else:
            key = base64.b64decode(value, validate=True)
    except (ValueError, binascii.Error):
        raise ValueError("Invalid configured network encryption key") from None
    if len(key) != SESSION_KEY_BYTES:
        raise ValueError("Invalid configured network encryption key length")
    return key


class MatchmakingService:
    """Ticket, websocket, session and key state for one backend instance."""

    def __init__(self, ctx, accounts, state_step_sec=STATE_STEP_SEC, websocket_origin=None, gameservers=None, oauth=None):
        self.ctx = ctx
        self.accounts = accounts
        self.oauth = oauth
        self.gameservers = gameservers
        self.state_step_sec = state_step_sec
        origin = websocket_origin or f"wss://{MATCHMAKING_HOST}"
        parts = urlsplit(origin)
        try:
            valid_port = parts.port is None or 1 <= parts.port <= 65535
        except ValueError:
            valid_port = False
        if (parts.scheme not in ("ws", "wss") or not parts.hostname or not valid_port or
                parts.username is not None or parts.password is not None or
                parts.path not in ("", "/") or parts.query or parts.fragment):
            raise ValueError("matchmaking websocket origin must be a ws:// or wss:// origin")
        self.websocket_origin = origin.rstrip("/")
        self._lock = threading.Lock()
        # [guess] Per-process secret used to HMAC the ticket payload. Never persisted.
        self._ticket_signing_key = secrets.token_bytes(32)
        self._network_key = _configured_network_key()
        self._tickets = {}
        self._sessions = {}
        self._session_keys = {}
        # Local client associations and views do not control dedicated-server admission.
        self._departed = set()
        self._client_views = {}

    def _session_record(self, session_id, owner_id, owner_name, players):
        return {
            "id": session_id,  # [observed] required session GUID
            "ownerId": owner_id,  # [guess] field value; name [observed] in binary
            "ownerName": owner_name,  # [guess]
            "serverName": "local-dedicated",  # [guess]
            "serverAddress": SESSION_SERVER_ADDRESS,  # [guess]
            "serverPort": SESSION_SERVER_PORT,  # [guess]
            "maxPublicPlayers": 100,  # [guess]
            "openPublicPlayers": 100 - len(players),  # [guess] available public slots
            "maxPrivatePlayers": 0,  # [observed] numeric session setting; value [guess]
            "openPrivatePlayers": 0,  # [observed] numeric session field; value [guess]
            "shouldAdvertise": True,  # [observed] boolean settings; values [guess]
            "allowJoinInProgress": True,
            "isDedicated": True,
            "usesStats": False,
            "allowInvites": False,
            "usesPresence": False,
            "allowJoinViaPresence": False,
            "allowJoinViaPresenceFriendsOnly": False,
            "publicPlayers": players,  # [guess] ticket party members
            "privatePlayers": [],  # [guess]
            "attributes": {"BeaconPort_i": SESSION_BEACON_PORT},  # [observed] integer suffix; value [guess]
            "buildUniqueId": BUILD_UNIQUE_ID,  # [observed] name; value [given]
            "started": False,  # [guess]
        }

    def ticket(self, account_id, query):
        """Answer GET .../matchmakingservice/ticket/player/{id}. Returns (status, body)."""
        # [observed] partyPlayerIds is sent in the ticket request. Its delimiter and
        # server-side roster rules are not recovered; use the comma-separated form
        # used by this local service, with the requesting player always included.
        party_ids = [value.strip() for value in query.get("partyPlayerIds", "").split(",")]
        players = list(dict.fromkeys([account_id] + [value for value in party_ids if value]))
        if len(players) > 100:
            raise ApiError(400, Err.INVALID_PAYLOAD, "The party exceeds the session capacity.")
        ticket_id = self.ctx.new_id("ticket")
        match_id = self.ctx.new_id("match")
        session_id = self.ctx.new_id("session") if self.gameservers is None else None
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
            bucket = query.get("bucketId", "").split(":")
            self._tickets[ticket_id] = {"matchId": match_id, "sessionId": session_id,
                                        "playerCount": len(players), "players": players,
                                        "playlist": bucket[3] if len(bucket) > 3 else "",
                                        "buildUniqueId": bucket[0] if bucket else ""}
            if self.gameservers is None:
                self._sessions[session_id] = self._session_record(session_id, account_id, owner_name, players)
        body = {  # [observed] field names per section 3; values [guess]
            "serviceUrl": f"{self.websocket_origin}{MATCHMAKING_WS_PATH}/{ticket_id}",
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
            if state == "ReadyCheck" and self.gameservers is not None:
                # [guess] Keep the ticket queued until a live server can admit the party.
                while True:
                    with self._lock:
                        if entry["sessionId"] is None:
                            entry["sessionId"] = self.gameservers.reserve(entry["playlist"], entry["players"], entry["buildUniqueId"])
                        assigned = entry["sessionId"]
                    if assigned is not None:
                        break
                    try:
                        conn.send_json(_message(STATUS_UPDATE_MESSAGE, state="Queued",
                                              totalPlayers=entry["playerCount"], connectedPlayers=1,
                                              queuedPlayers=entry["playerCount"], estimatedWaitSec=1,
                                              ticketId=ticket_id, matchId=entry["matchId"]))
                    except (local_ws.WebSocketClosed, OSError):
                        return
                    time.sleep(max(self.state_step_sec, 0.25))
            count = entry["playerCount"]
            queued = count if state in ("Queued", "ReadyCheck", "SessionAssignment") else 0
            fields = {
                "state": state,  # [observed] state names
                "totalPlayers": count, "connectedPlayers": 1,  # local ticket roster and socket
                "queuedPlayers": queued, "estimatedWaitSec": 0,  # [guess] values
                "ticketId": ticket_id, "matchId": entry["matchId"],
            }
            if state == "ReadyCheck":
                # [observed] reader fields; locally the entire ticket roster is accepted.
                fields.update(status="ACTIVE", durationSec=0, ready=True, responded=True,
                              readyPlayers=count, unreadyPlayers=0)
            conn.send_json(_message(STATUS_UPDATE_MESSAGE, **fields))
            if self.state_step_sec:
                time.sleep(self.state_step_sec)
        conn.send_json(_message(
            PLAY_MESSAGE,
            matchId=entry["matchId"],  # [observed] field name
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
        if self.gameservers is not None:
            return self.gameservers.session(session_id)
        with self._lock:
            record = self._sessions.get(session_id)
            if record is None:
                return None
            return 200, dict(record, publicPlayers=list(record["publicPlayers"]),
                             privatePlayers=list(record["privatePlayers"]),
                             attributes=dict(record["attributes"]))

    def join(self, session_id, account_id):
        """Acknowledge a join for a ticket roster member of an existing session.

        The client sends an empty POST and does not parse a success body. Roster
        admission and the 204 response are local policies; this does not authenticate
        the accountId query parameter or prove the game-server connection succeeded.
        """
        if self.gameservers is not None:
            result = self.gameservers.join(session_id, account_id)
        else:
            with self._lock:
                session = self._sessions.get(session_id)
                result = ((204, None) if session and account_id in
                          session["publicPlayers"] + session["privatePlayers"] else None)
        if result is not None:
            with self._lock:
                self._departed.discard((session_id, account_id))
        return result

    def client_operation(self, req, session_id, operation):
        """Local policy for client session views; never alter server-owned admission.

        The observed client sends these requests on its joined game session. 204 is
        a local response policy; the original backend's response is not recovered.
        """
        account_id = self.oauth.account_session(req).account_id
        result = self.session(session_id)
        if result is None:
            return None
        record = result[1]
        roster = record["publicPlayers"] + record["privatePlayers"]
        if account_id not in roster:
            return None
        key = (session_id, account_id)
        with self._lock:
            if key in self._departed:
                return None
            if operation == "players":
                body = req.json_object()
                public, private = body.get("publicPlayers"), body.get("privatePlayers")
                if (not isinstance(public, list) or not isinstance(private, list) or
                        any(not isinstance(p, str) or p not in roster for p in public + private) or
                        account_id not in public + private or len(public + private) != len(set(public + private))):
                    raise ApiError(400, Err.INVALID_PAYLOAD, "Invalid session player registration.")
                self._client_views[key] = {"publicPlayers": list(public), "privatePlayers": list(private)}
            elif operation == "update":
                body = req.json_object()
                if body.get("id") != session_id:
                    raise ApiError(400, Err.INVALID_PAYLOAD, "Session ID does not match the request.")
                # Preserve only the submitting client's view; canonical server metadata
                # and admission roster remain untouched.
                self._client_views[key] = body
            elif operation == "destroy":
                self._client_views.pop(key, None)
                self._departed.add(key)
        return 204, None

    def session_key(self, account_id, session_id):
        """Answer GET .../matchmaking/account/{acct}/session/{sid}. None if the session is unknown.

        Use the dedicated server's configured key when present; otherwise create
        a random 32-byte key on first request. Keys are kept in memory only.
        """
        if self.gameservers is not None:
            return self.gameservers.session_key(account_id, session_id)
        with self._lock:
            session = self._sessions.get(session_id)
            if session is None or account_id not in session["publicPlayers"] + session["privatePlayers"]:
                return None
            key = self._session_keys.get(session_id)
            if key is None:
                key = self._network_key or secrets.token_bytes(SESSION_KEY_BYTES)
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
        for method, suffix, operation in (("POST", "", "update"), ("DELETE", "", "destroy"),
                                           ("POST", "/players", "players"),
                                           ("POST", "/heartbeat", "heartbeat")):
            router.add(method, r"(?:/fortnite)?/api/matchmaking/session/([^/]+)" + suffix,
                       lambda req, match, op=operation: _found(self.client_operation(req, match.group(1), op),
                                                               match.group(0)),
                       name="matchmaking.client_" + operation, status="inferred")
        router.add("POST", r"(?:/fortnite)?/api/matchmaking/session/([^/]+)/join",
                   lambda req, match: _found(self.join(match.group(1), req.query.get("accountId", "")),
                                             match.group(0)),
                   name="matchmaking.join", status="observed")
        router.add("GET", r"(?:/fortnite)?/api/game/v2/matchmaking/account/([^/]+)/session/([^/]+)",
                   lambda req, match: _found(self.session_key(match.group(1), match.group(2)), match.group(0)),
                   name="matchmaking.session_key", status="observed")


def _found(result, path):
    """Unwrap a (status, body) pair, or answer 404 when the lookup returned None."""
    if result is None:
        raise ApiError(404, Err.NOT_FOUND, f"Sorry, the resource {path} was not found", numeric=1004)
    return result
