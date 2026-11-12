"""Party v1 and presence endpoints (local stand-in). Parties and presence live in memory only.

Evidence (see the 12.60 binary strings and DefaultEngine.ini):
  observed  the route templates /api/v1/{namespace}/parties, .../parties/{partyId},
            .../members/{accountId}[/join|/meta|/promote|/confirm|/reject|/disconnect|
            /conferences/connection], .../invites/{accountId}[/{fromAccountId}/decline],
            /api/v1/{namespace}/user/{accountId}/pings/{fromAccountId}/parties; the
            field names revision, members, invites, pings, current, pending, joinability,
            discoverability, join_info, connections, connection, joined_at, updated_at,
            role, CAPTAIN, delete, update, sendPing, ttl, maxSize; the config keys
            ServiceName="party" and bUsePartySystemV2 (so the party prefix is /party);
            the error codes party_not_found, member_not_found, party_is_full,
            party_join_forbidden, invite_not_found, stale_revision, self_ping and others;
            the presence template /presence/api/v1/_/{id}/last-online and
            /presence/api/v1/_/{id}/subscriptions/{otherId}.
  inferred  the HTTP methods, the /party/api/v1/Fortnite prefix (ServiceName is
            observed, the namespace value is inferred), request body shapes,
            the response shapes, the build-id check (urn:epic:cfg:build-id_s) and the
            CLOSED joinability value.
  guess     the PATCH /parties/{partyId}/meta route, and the presence response shape.

Not implemented: party chat, join requests that wait for approval
(confirm and reject answer 404 applicant_not_found because no request can exist),
voice conferences
(the /conferences/connection route is not registered, so it answers 404).
"""

import copy

from api_common import ApiError, Err

PARTY_PREFIX = "/party/api/v1/Fortnite"
PRESENCE_PREFIX = "/presence/api/v1/_"
# inferred: the build-id key that party join compares. Absent from the binary as a plain string.
BUILD_KEY = "urn:epic:cfg:build-id_s"
JOINABILITY = ("OPEN", "CLOSED", "INVITE_AND_FORMER")  # last value observed in 12.60 create
MAX_PARTY_SIZE = 16  # guess: default maxSize
INVITE_TTL_SEC = 14400  # guess
PING_TTL_SEC = 60  # guess
DEFAULT_CONFIG = {
    "type": "DEFAULT",  # observed
    "joinability": "OPEN",  # observed key; value inferred
    "discoverability": "ALL",  # observed key; value inferred
    "subType": "default",  # observed key
    "maxSize": MAX_PARTY_SIZE,  # observed key
    "invite_ttl_seconds": INVITE_TTL_SEC,  # observed key
    "intentions_ttl_seconds": 60,  # observed key
}


class PartyService:
    def __init__(self, ctx, accounts, oauth, *, notify=None):
        self.ctx = ctx
        self.accounts = accounts
        self.oauth = oauth
        self.notify = notify
        self.notification_failures = 0
        self._parties = {}
        self._presence = {}
        self._subscriptions = {}
        self._pings = []

    # Helpers -----------------------------------------------------------

    def _emit(self, party, kind, actor, recipients=None, **fields):
        # Notification names and fields are inferred, not captured from the client.
        if not self.notify:
            return
        event = {"type": "com.epicgames.social.party.notification.v0." + kind,
                 "party_id": party["id"], "account_id": actor,
                 "revision": party["revision"], **copy.deepcopy(fields)}
        if recipients is None:
            recipients = [member["account_id"] for member in party["members"]]
        for recipient in set(recipients):
            try:
                self.notify(recipient, copy.deepcopy(event))
            except Exception:
                self.notification_failures += 1

    def _caller(self, req):
        return self.oauth.account_session(req).account_id

    def _require_party(self, party_id):
        party = self._parties.get(party_id)
        if party is None:
            raise ApiError(404, Err.PARTY_NOT_FOUND, "The party was not found.")
        self._expire_invites(party)
        return party

    def _expire_invites(self, party):
        now = self.ctx.iso()
        active = [invite for invite in party["invites"] if invite["expires_at"] > now]
        if len(active) != len(party["invites"]):
            party["invites"] = active
            self._touch(party)

    @staticmethod
    def _member(party, account_id):
        for member in party["members"]:
            if member["account_id"] == account_id:
                return member
        return None

    @staticmethod
    def _captain(party):
        for member in party["members"]:
            if member["role"] == "CAPTAIN":
                return member
        return None

    def _party_of(self, account_id):
        for party in self._parties.values():
            if self._member(party, account_id) is not None:
                return party
        return None

    def _touch(self, party):
        party["revision"] += 1
        party["updated_at"] = self.ctx.iso()

    def _set_presence(self, account_id, status, party_id):
        self._presence[account_id] = {
            "status": status,
            "partyId": party_id,
            "lastOnline": self.ctx.iso(),
        }

    def _new_member(self, account_id, join_info, role):
        now = self.ctx.iso()
        connection = self._dict(join_info.get("connection"), "connection")
        member_meta = self._dict(join_info.get("meta"), "join_info.meta")
        connection_meta = self._dict(connection.get("meta"), "connection.meta")
        return {
            "account_id": account_id,
            "meta": copy.deepcopy(member_meta),
            "connections": [{
                "id": self._connection_id(join_info, account_id),
                "connected_at": now,
                "updated_at": now,
                "meta": copy.deepcopy(connection_meta),
                "yield_leadership": False,
            }],
            "revision": 0,
            "updated_at": now,
            "joined_at": now,
            "role": role,
        }

    @staticmethod
    def _dict(value, label):
        if value is None:
            return {}
        if not isinstance(value, dict):
            raise ApiError(400, Err.INVALID_PAYLOAD, f"{label} must be an object.")
        return value

    @staticmethod
    def _connection_id(body, account_id):
        connection = body.get("connection") or {}
        if isinstance(connection, dict) and isinstance(connection.get("id"), str) and connection["id"]:
            return connection["id"]
        return f"{account_id}:connection"

    def _config(self, value):
        config = dict(DEFAULT_CONFIG)
        for key, item in self._dict(value, "config").items():
            if key in DEFAULT_CONFIG or key == "join_confirmation":
                config[key] = item
        if config["joinability"] not in JOINABILITY:
            raise ApiError(400, Err.INVALID_PAYLOAD, "Invalid joinability.")
        if "join_confirmation" in config and not isinstance(config["join_confirmation"], bool):
            raise ApiError(400, Err.INVALID_PAYLOAD, "join_confirmation must be a boolean.")
        size = config["maxSize"]
        if isinstance(size, bool) or not isinstance(size, int) or not 1 <= size <= MAX_PARTY_SIZE:
            raise ApiError(400, Err.INVALID_PAYLOAD, f"maxSize must be between 1 and {MAX_PARTY_SIZE}.")
        ttl = config["invite_ttl_seconds"]
        if isinstance(ttl, bool) or not isinstance(ttl, int) or ttl < 1:
            raise ApiError(400, Err.INVALID_PAYLOAD, "invite_ttl_seconds must be a positive integer.")
        return config

    @staticmethod
    def _meta_body(body):
        deletes = body.get("delete", [])
        updates = body.get("update", {})
        if not isinstance(deletes, list) or not all(isinstance(key, str) for key in deletes):
            raise ApiError(400, Err.INVALID_PAYLOAD, "delete must be a list of keys.")
        if not isinstance(updates, dict) or not all(isinstance(key, str) for key in updates):
            raise ApiError(400, Err.INVALID_PAYLOAD, "update must be an object.")
        return deletes, updates

    @staticmethod
    def _check_revision(body, current):
        if "revision" in body and body["revision"] != current:
            raise ApiError(409, Err.STALE_REVISION, "The revision is stale. Fetch the latest state and retry.")

    # Party handlers ----------------------------------------------------

    def create(self, req, match):
        caller = self._caller(req)
        if self._party_of(caller) is not None:
            raise ApiError(409, Err.USER_HAS_PARTY, "The user is already in a party.")
        body = req.json_object()
        join_info = self._dict(body.get("join_info"), "join_info")
        party_id = self.ctx.new_id("party")
        now = self.ctx.iso()
        party = {
            "id": party_id,
            "created_at": now,
            "updated_at": now,
            "revision": 0,
            "config": self._config(body.get("config")),
            "meta": self._dict(body.get("meta"), "meta"),
            "members": [],
            "invites": [],
        }
        party["members"].append(self._new_member(caller, join_info, "CAPTAIN"))
        party["revision"] = 1
        self._parties[party_id] = party
        self._set_presence(caller, "Online", party_id)
        self._emit(party, "MEMBER_JOINED", caller, member=party["members"][-1])
        return 200, copy.deepcopy(party)

    def get(self, req, match):
        self._caller(req)
        return 200, copy.deepcopy(self._require_party(match.group(1)))

    def join(self, req, match):
        party_id, account_id = match.group(1), match.group(2)
        caller = self._caller(req)
        if caller != account_id:
            raise ApiError(403, Err.MISSING_PERMISSION, "Only the account itself can join a party for it.")
        party = self._require_party(party_id)
        current = self._party_of(account_id)
        if current is not None:
            if current["id"] == party_id:
                raise ApiError(409, Err.USER_ALREADY_IN_PARTY, "The user is already in this party.")
            raise ApiError(409, Err.USER_HAS_PARTY, "The user is already in another party.")
        body = req.json_object()
        invite = self._find_invite(party, account_id)
        if len(party["members"]) >= party["config"]["maxSize"]:
            raise ApiError(409, Err.PARTY_IS_FULL, "The party is full.")
        expected_build = party["meta"].get(BUILD_KEY)
        if expected_build is not None and self._dict(body.get("meta"), "meta").get(BUILD_KEY) != expected_build:
            raise ApiError(403, Err.PARTY_JOIN_FORBIDDEN, "The build does not match this party.")
        if (party["config"]["joinability"] != "OPEN" or party["config"].get("join_confirmation", False)) and invite is None:
            raise ApiError(403, Err.PARTY_JOIN_FORBIDDEN, "An invite is required to join this party.")
        member = self._new_member(account_id, body, "MEMBER")
        if invite is not None:
            party["invites"].remove(invite)
        party["members"].append(member)
        self._touch(party)
        self._set_presence(account_id, "Online", party_id)
        self._emit(party, "MEMBER_JOINED", account_id, member=party["members"][-1])
        return 200, copy.deepcopy(party)

    def leave_or_kick(self, req, match):
        party_id, account_id = match.group(1), match.group(2)
        caller = self._caller(req)
        party = self._require_party(party_id)
        if self._member(party, account_id) is None:
            raise ApiError(404, Err.PARTY_MEMBER_NOT_FOUND, "The member was not found in the party.")
        if caller != account_id:
            captain = self._captain(party)
            if captain is None or captain["account_id"] != caller:
                raise ApiError(403, Err.USER_OPERATION_FORBIDDEN, "Only the party captain can remove other members.")
        recipients = [member["account_id"] for member in party["members"]]
        was_captain = self._member(party, account_id)["role"] == "CAPTAIN"
        party["members"] = [member for member in party["members"] if member["account_id"] != account_id]
        self._set_presence(account_id, "Offline", None)
        if not party["members"]:
            del self._parties[party_id]
            self._pings = [ping for ping in self._pings if ping["party_id"] != party_id]
            self._emit(party, "MEMBER_LEFT", account_id, recipients=recipients)
            return 204, None
        if was_captain and self._captain(party) is None:
            oldest = min(party["members"], key=lambda member: member["joined_at"])
            oldest["role"] = "CAPTAIN"
        self._touch(party)
        self._emit(party, "MEMBER_LEFT", account_id, recipients=recipients)
        return 204, None

    def _find_invite(self, party, account_id):
        for invite in party["invites"]:
            if invite["account_id"] == account_id:
                return invite
        return None

    def invite(self, req, match):
        party_id, target = match.group(1), match.group(2)
        caller = self._caller(req)
        party = self._require_party(party_id)
        if self._member(party, caller) is None:
            raise ApiError(403, Err.INVITE_FORBIDDEN, "Only party members can send invites.")
        if self.accounts.get(target) is None:
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, "The invited account was not found.")
        if self._member(party, target) is not None:
            raise ApiError(409, Err.USER_ALREADY_IN_PARTY, "The user is already in this party.")
        if self._find_invite(party, target) is not None:
            raise ApiError(409, Err.INVITE_ALREADY_EXISTS, "An invite for this user already exists.")
        body = req.json_object()
        ttl = body.get("ttl", party["config"]["invite_ttl_seconds"])
        if isinstance(ttl, bool) or not isinstance(ttl, int) or ttl < 1:
            raise ApiError(400, Err.INVALID_PAYLOAD, "ttl must be a positive integer.")
        meta = self._dict(body.get("meta"), "meta")
        now = self.ctx.now()
        party["invites"].append({
            "account_id": target,
            "sent_by": caller,
            "meta": meta,
            "sent_at": self.ctx.iso(now),
            "expires_at": self.ctx.iso(now + ttl),
        })
        self._touch(party)
        if body.get("sendPing") is True:
            self._add_ping(caller, target, party_id, meta)
            self._emit(party, "PING", caller, recipients=[target], meta=meta)
        self._emit(party, "INITIAL_INVITE", caller, recipients=[target], invite=party["invites"][-1])
        return 204, None

    def decline_invite(self, req, match):
        party_id, account_id, sender = match.group(1), match.group(2), match.group(3)
        caller = self._caller(req)
        if caller != account_id:
            raise ApiError(403, Err.MISSING_PERMISSION, "Only the invited account can decline the invite.")
        party = self._require_party(party_id)
        for invite in party["invites"]:
            if invite["account_id"] == account_id and invite["sent_by"] == sender:
                party["invites"].remove(invite)
                self._touch(party)
                self._emit(party, "INVITE_DECLINED", account_id, recipients=[sender])
                return 204, None
        raise ApiError(404, Err.INVITE_NOT_FOUND, "The invite was not found.")

    def member_meta(self, req, match):
        party_id, account_id = match.group(1), match.group(2)
        caller = self._caller(req)
        if caller != account_id:
            raise ApiError(403, Err.MEMBER_STATE_FORBIDDEN, "Only the member can change its own metadata.")
        party = self._require_party(party_id)
        member = self._member(party, account_id)
        if member is None:
            raise ApiError(404, Err.PARTY_MEMBER_NOT_FOUND, "The member was not found in the party.")
        body = req.json_object()
        self._check_revision(body, member["revision"])
        deletes, updates = self._meta_body(body)
        for key in deletes:
            member["meta"].pop(key, None)
        member["meta"].update(updates)
        member["revision"] += 1
        member["updated_at"] = self.ctx.iso()
        self._touch(party)
        self._emit(party, "MEMBER_STATE_UPDATED", account_id, member=member)
        return 204, None

    def party_meta(self, req, match):
        party_id = match.group(1)
        caller = self._caller(req)
        party = self._require_party(party_id)
        captain = self._captain(party)
        if captain is None or captain["account_id"] != caller:
            raise ApiError(403, Err.PARTY_CHANGE_FORBIDDEN, "Only the party captain can change party metadata.")
        body = req.json_object()
        self._check_revision(body, party["revision"])
        deletes, updates = self._meta_body(body)
        for key in deletes:
            party["meta"].pop(key, None)
        party["meta"].update(updates)
        self._touch(party)
        self._emit(party, "PARTY_UPDATED", caller, config=party["config"], meta=party["meta"])
        return 204, None

    def promote(self, req, match):
        party_id, account_id = match.group(1), match.group(2)
        caller = self._caller(req)
        party = self._require_party(party_id)
        captain = self._captain(party)
        if captain is None or captain["account_id"] != caller:
            raise ApiError(403, Err.MEMBER_STATE_FORBIDDEN, "Only the party captain can promote a member.")
        target = self._member(party, account_id)
        if target is None:
            raise ApiError(404, Err.PARTY_MEMBER_NOT_FOUND, "The member was not found in the party.")
        for member in party["members"]:
            member["role"] = "CAPTAIN" if member is target else "MEMBER"
        self._touch(party)
        self._emit(party, "MEMBER_NEW_CAPTAIN", account_id)
        return 204, None

    def applicant_decision(self, req, match):
        party_id, account_id = match.group(1), match.group(2)
        caller = self._caller(req)
        party = self._require_party(party_id)
        captain = self._captain(party)
        if captain is None or captain["account_id"] != caller:
            raise ApiError(403, Err.MEMBER_STATE_FORBIDDEN, "Only the party captain can decide join requests.")
        raise ApiError(404, Err.APPLICANT_NOT_FOUND,
                       f"No pending join request exists for {account_id}.")

    def disconnect(self, req, match):
        party_id, account_id = match.group(1), match.group(2)
        caller = self._caller(req)
        if caller != account_id:
            raise ApiError(403, Err.MISSING_PERMISSION, "Only the member can report its own disconnect.")
        party = self._require_party(party_id)
        member = self._member(party, account_id)
        if member is None:
            raise ApiError(404, Err.PARTY_MEMBER_NOT_FOUND, "The member was not found in the party.")
        member["connections"] = []
        member["revision"] += 1
        member["updated_at"] = self.ctx.iso()
        self._set_presence(account_id, "Offline", None)
        self._touch(party)
        self._emit(party, "MEMBER_STATE_UPDATED", account_id, member=member)
        return 204, None

    # User and ping handlers -------------------------------------------

    def user_state(self, req, match):
        account_id = match.group(1)
        caller = self._caller(req)
        if caller != account_id:
            raise ApiError(403, Err.MISSING_PERMISSION, "A user can only read its own party state.")
        for party in self._parties.values():
            self._expire_invites(party)
        now = self.ctx.iso()
        self._pings = [ping for ping in self._pings if ping["expires_at"] > now]
        current = [copy.deepcopy(party) for party in self._parties.values()
                   if self._member(party, account_id) is not None]
        invites = []
        for party in self._parties.values():
            for invite in party["invites"]:
                if invite["account_id"] == account_id:
                    invites.append({"party_id": party["id"], **copy.deepcopy(invite)})
        pings = [copy.deepcopy(ping) for ping in self._pings if ping["account_id"] == account_id]
        return 200, {"current": current, "pending": [], "invites": invites, "pings": pings}

    def _add_ping(self, sender, target, party_id, meta):
        self._pings = [ping for ping in self._pings
                       if not (ping["sent_by"] == sender and ping["account_id"] == target)]
        now = self.ctx.now()
        self._pings.append({
            "account_id": target,
            "sent_by": sender,
            "party_id": party_id,
            "meta": dict(meta),
            "sent_at": self.ctx.iso(now),
            "expires_at": self.ctx.iso(now + PING_TTL_SEC),
        })

    def send_ping(self, req, match):
        target, sender = match.group(1), match.group(2)
        caller = self._caller(req)
        if caller != sender:
            raise ApiError(403, Err.MISSING_PERMISSION, "A ping can only be sent as the signed-in account.")
        if target == sender:
            raise ApiError(400, Err.SELF_PING, "A ping cannot be sent to yourself.")
        if self.accounts.get(target) is None:
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, "The account was not found.")
        party = self._party_of(sender)
        if party is None:
            raise ApiError(403, Err.USER_OPERATION_FORBIDDEN, "The sender must be in a party to send a ping.")
        body = req.json_object()
        meta = self._dict(body.get("meta"), "meta")
        self._add_ping(sender, target, party["id"], meta)
        self._emit(party, "PING", sender, recipients=[target], meta=meta)
        return 204, None

    # Presence handlers -------------------------------------------------

    def last_online(self, req, match):
        account_id = match.group(1)
        self._caller(req)
        if self.accounts.get(account_id) is None:
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, "The account was not found.")
        entry = self._presence.get(account_id, {"status": "Offline", "partyId": None, "lastOnline": None})
        return 200, {"accountId": account_id, **entry}

    def list_subscriptions(self, req, match):
        account_id = match.group(1)
        if self._caller(req) != account_id:
            raise ApiError(403, Err.MISSING_PERMISSION, "A user can only read its own subscriptions.")
        return 200, sorted(self._subscriptions.get(account_id, set()))

    def add_subscription(self, req, match):
        account_id, other = match.group(1), match.group(2)
        if self._caller(req) != account_id:
            raise ApiError(403, Err.MISSING_PERMISSION, "A user can only change its own subscriptions.")
        if self.accounts.get(other) is None:
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, "The account was not found.")
        self._subscriptions.setdefault(account_id, set()).add(other)
        return 204, None

    def remove_subscription(self, req, match):
        account_id, other = match.group(1), match.group(2)
        if self._caller(req) != account_id:
            raise ApiError(403, Err.MISSING_PERMISSION, "A user can only change its own subscriptions.")
        self._subscriptions.get(account_id, set()).discard(other)
        return 204, None

    def register(self, router):
        P, R = PARTY_PREFIX, PRESENCE_PREFIX
        router.add("POST", rf"{P}/parties", self.create, name="party.create", status="inferred")
        router.add("GET", rf"{P}/parties/([^/]+)", self.get, name="party.get", status="inferred")
        router.add("POST", rf"{P}/parties/([^/]+)/members/([^/]+)/join", self.join,
                   name="party.member.join", status="inferred")
        router.add("DELETE", rf"{P}/parties/([^/]+)/members/([^/]+)", self.leave_or_kick,
                   name="party.member.leave_or_kick", status="inferred")
        router.add("POST", rf"{P}/parties/([^/]+)/invites/([^/]+)", self.invite,
                   name="party.invite", status="inferred")
        router.add("POST", rf"{P}/parties/([^/]+)/invites/([^/]+)/([^/]+)/decline", self.decline_invite,
                   name="party.invite.decline", status="inferred")
        router.add("PATCH", rf"{P}/parties/([^/]+)/members/([^/]+)/meta", self.member_meta,
                   name="party.member.meta", status="inferred")
        router.add("PATCH", rf"{P}/parties/([^/]+)/meta", self.party_meta,
                   name="party.meta", status="guess")
        router.add("POST", rf"{P}/parties/([^/]+)/members/([^/]+)/promote", self.promote,
                   name="party.member.promote", status="inferred")
        router.add("POST", rf"{P}/parties/([^/]+)/members/([^/]+)/(confirm|reject)", self.applicant_decision,
                   name="party.member.applicant", status="inferred")
        router.add("POST", rf"{P}/parties/([^/]+)/members/([^/]+)/disconnect", self.disconnect,
                   name="party.member.disconnect", status="inferred")
        router.add("GET", rf"{P}/user/([^/]+)", self.user_state, name="party.user", status="inferred")
        router.add("POST", rf"{P}/user/([^/]+)/pings/([^/]+)/parties", self.send_ping,
                   name="party.ping", status="inferred")
        router.add("GET", rf"{R}/([^/]+)/last-online", self.last_online,
                   name="presence.last_online", status="inferred")
        router.add("GET", rf"{R}/([^/]+)/subscriptions", self.list_subscriptions,
                   name="presence.subscriptions.list", status="inferred")
        router.add("POST", rf"{R}/([^/]+)/subscriptions/([^/]+)", self.add_subscription,
                   name="presence.subscriptions.add", status="inferred")
        router.add("DELETE", rf"{R}/([^/]+)/subscriptions/([^/]+)", self.remove_subscription,
                   name="presence.subscriptions.remove", status="inferred")
