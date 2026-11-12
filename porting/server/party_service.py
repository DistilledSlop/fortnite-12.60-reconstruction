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

Local XMPP party chat uses an inferred room naming convention; the original client
room naming is unverified. Voice conferences are not implemented
(the /conferences/connection route is not registered, so it answers 404).
"""

import copy

from api_common import ApiError, Err

PARTY_PREFIX = "/party/api/v1/Fortnite"
PRESENCE_PREFIX = "/presence/api/v1/_"
# inferred: the build-id key that party join compares. Absent from the binary as a plain string.
BUILD_KEY = "urn:epic:cfg:build-id_s"
JOINABILITY = ("OPEN", "CLOSED", "INVITE_ONLY", "INVITE_AND_FORMER")
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
        # Applicants are intentionally separate from the public party/member state.
        self._applicants = {}
        self._presence = {}
        self._subscriptions = {}
        self._pings = []

    # Helpers -----------------------------------------------------------

    def _emit(self, party, kind, actor, recipients=None, **fields):
        if not self.notify:
            return
        event = {"type": "com.epicgames.social.party.notification.v0." + kind,
                 "party_id": party["id"]}
        if kind == "PARTY_UPDATED":
            config = party["config"]
            # CLOSED is a legacy local value; the client recognizes INVITE_ONLY.
            privacy = "INVITE_ONLY" if config["joinability"] == "CLOSED" else config["joinability"]
            captain = self._captain(party)
            event.update({"ns": "Fortnite", "sent": self.ctx.iso(),
                          "captain_id": captain["account_id"] if captain else "",
                          "party_privacy_type": privacy, "max_number_of_members": config["maxSize"],
                          "party_type": config["type"] if isinstance(config["type"], str) else DEFAULT_CONFIG["type"],
                          "party_sub_type": config["subType"] if isinstance(config["subType"], str) else DEFAULT_CONFIG["subType"],
                          "revision": party["revision"], **copy.deepcopy(fields)})
        elif kind in ("MEMBER_STATE_UPDATED", "MEMBER_JOINED", "MEMBER_REJECTED", "MEMBER_REQUIRE_CONFIRMATION"):
            member = fields.pop("member", None) or self._member(party, actor)
            include_connection = fields.pop("include_connection", kind == "MEMBER_JOINED")
            account = self.accounts.get(actor)
            event.update({"ns": "Fortnite", "sent": self.ctx.iso(), "account_id": actor,
                          "account_dn": account["displayName"] if account else "",
                          "revision": member["revision"], "reason": "",
                          "member_state_removed": [],
                          "member_state_updated": copy.deepcopy(member["meta"]),
                          **copy.deepcopy(fields)})
            if include_connection and member["connections"]:
                connection = member["connections"][0]
                event["connection"] = {"id": connection["id"], "disconnected_at": "",
                                       "meta": copy.deepcopy(connection["meta"])}
        else:
            # Other notification formats remain inferred.
            event.update({"account_id": actor, "revision": party["revision"], **copy.deepcopy(fields)})
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
        self._expire_applicants(party)
        return party

    def _expire_invites(self, party):
        now = self.ctx.iso()
        active = [invite for invite in party["invites"] if invite["expires_at"] > now]
        if len(active) != len(party["invites"]):
            party["invites"] = active
            self._touch(party)

    def _expire_applicants(self, party):
        pending = self._applicants.get(party["id"], {})
        now = self.ctx.iso()
        for account_id in list(pending):
            if pending[account_id]["expires_at"] <= now:
                del pending[account_id]

    def _clear_applicant(self, account_id):
        for pending in self._applicants.values():
            pending.pop(account_id, None)

    def _admit(self, party, account_id, member):
        invite = self._find_invite(party, account_id)
        if invite is not None:
            party["invites"].remove(invite)
        self._clear_applicant(account_id)
        party["members"].append(member)
        self._touch(party)
        self._set_presence(account_id, "Online", party["id"])
        self._emit(party, "MEMBER_JOINED", account_id, member=member)

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

    def _invite_expiry(self, ttl, label):
        if isinstance(ttl, bool) or not isinstance(ttl, int) or ttl < 1:
            raise ApiError(400, Err.INVALID_PAYLOAD, f"{label} must be a positive integer.")
        try:
            return self.ctx.iso(self.ctx.now() + ttl)
        except (OverflowError, OSError, ValueError):
            raise ApiError(400, Err.INVALID_PAYLOAD, f"{label} produces an unrepresentable expiry.")

    def _config(self, value, base=None):
        config = dict(DEFAULT_CONFIG if base is None else base)
        supplied = self._dict(value, "config")
        for key in ("maxSize", "max_size"):
            if key in supplied:
                size = supplied[key]
                if isinstance(size, bool) or not isinstance(size, int) or not 1 <= size <= MAX_PARTY_SIZE:
                    raise ApiError(400, Err.INVALID_PAYLOAD, f"{key} must be between 1 and {MAX_PARTY_SIZE}.")
        if "max_size" in supplied and "maxSize" in supplied and supplied["maxSize"] != supplied["max_size"]:
            raise ApiError(400, Err.INVALID_PAYLOAD, "maxSize and max_size must agree.")
        for key, item in supplied.items():
            if key in DEFAULT_CONFIG or key in ("join_confirmation", "max_size", "invite_ttl"):
                config[key] = item
        if "maxSize" in supplied and "max_size" not in supplied and "max_size" in config:
            config["max_size"] = supplied["maxSize"]
        if "max_size" in supplied:
            config["maxSize"] = supplied["max_size"]
        for key in ("invite_ttl", "invite_ttl_seconds"):
            if key in supplied:
                self._invite_expiry(supplied[key], key)
        if "invite_ttl" in supplied and "invite_ttl_seconds" in supplied and supplied["invite_ttl"] != supplied["invite_ttl_seconds"]:
            raise ApiError(400, Err.INVALID_PAYLOAD, "invite_ttl and invite_ttl_seconds must agree.")
        if "invite_ttl" in supplied:
            config["invite_ttl_seconds"] = supplied["invite_ttl"]
        elif "invite_ttl_seconds" in supplied and "invite_ttl" in config:
            config["invite_ttl"] = supplied["invite_ttl_seconds"]
        if config["joinability"] not in JOINABILITY:
            raise ApiError(400, Err.INVALID_PAYLOAD, "Invalid joinability.")
        if config["discoverability"] not in ("ALL", "INVITED_ONLY"):
            raise ApiError(400, Err.INVALID_PAYLOAD, "Invalid discoverability.")
        if "join_confirmation" in config and not isinstance(config["join_confirmation"], bool):
            raise ApiError(400, Err.INVALID_PAYLOAD, "join_confirmation must be a boolean.")
        for key in ("maxSize", "max_size"):
            if key in config:
                size = config[key]
                if isinstance(size, bool) or not isinstance(size, int) or not 1 <= size <= MAX_PARTY_SIZE:
                    raise ApiError(400, Err.INVALID_PAYLOAD, f"{key} must be between 1 and {MAX_PARTY_SIZE}.")
        self._invite_expiry(config["intentions_ttl_seconds"], "intentions_ttl_seconds")
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
            "meta": copy.deepcopy(self._dict(body.get("meta"), "meta")),
            "members": [],
            "invites": [],
        }
        party["members"].append(self._new_member(caller, join_info, "CAPTAIN"))
        party["revision"] = 1
        self._parties[party_id] = party
        self._clear_applicant(caller)
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
        # Constructing the member validates nested objects before any admission or request.
        member = self._new_member(account_id, body, "MEMBER")
        invite = self._find_invite(party, account_id)
        if len(party["members"]) >= party["config"]["maxSize"]:
            raise ApiError(409, Err.PARTY_IS_FULL, "The party is full.")
        expected_build = party["meta"].get(BUILD_KEY)
        if expected_build is not None and member["meta"].get(BUILD_KEY) != expected_build:
            raise ApiError(403, Err.PARTY_JOIN_FORBIDDEN, "The build does not match this party.")
        if party["config"]["joinability"] != "OPEN" and invite is None:
            raise ApiError(403, Err.PARTY_JOIN_FORBIDDEN, "An invite is required to join this party.")
        if party["config"].get("join_confirmation", False):
            expiry = self._invite_expiry(party["config"]["intentions_ttl_seconds"], "intentions_ttl_seconds")
            self._applicants.setdefault(party_id, {})[account_id] = {
                "member": member, "expires_at": expiry,
                "has_connection": isinstance(body.get("connection"), dict),
            }
            captain = self._captain(party)
            self._emit(party, "MEMBER_REQUIRE_CONFIRMATION", account_id,
                       recipients=[captain["account_id"]], member=member,
                       include_connection=isinstance(body.get("connection"), dict))
            return 200, {"party_id": party_id, "status": "PENDING_CONFIRMATION"}
        self._admit(party, account_id, member)
        return 200, {"party_id": party_id, "status": "JOINED"}

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
            self._applicants.pop(party_id, None)
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
        expires_at = self._invite_expiry(ttl, "ttl")
        meta = self._dict(body.get("meta"), "meta")
        now = self.ctx.now()
        party["invites"].append({
            "account_id": target,
            "sent_by": caller,
            "meta": meta,
            "sent_at": self.ctx.iso(now),
            "expires_at": expires_at,
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
        member["meta"].update(copy.deepcopy(updates))
        member["revision"] += 1
        member["updated_at"] = self.ctx.iso()
        self._touch(party)
        self._emit(party, "MEMBER_STATE_UPDATED", caller,
                   member_state_removed=deletes, member_state_updated=updates)
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
        party["meta"].update(copy.deepcopy(updates))
        self._touch(party)
        self._emit(party, "PARTY_UPDATED", caller, party_state_removed=deletes,
                   party_state_updated=updates, party_state_overrode={})
        return 204, None

    def party_update(self, req, match):
        caller = self._caller(req)
        party = self._require_party(match.group(1))
        captain = self._captain(party)
        if captain is None or captain["account_id"] != caller:
            raise ApiError(403, Err.PARTY_CHANGE_FORBIDDEN, "Only the party captain can change the party.")
        body = req.json_object()
        revision = body.get("revision")
        if isinstance(revision, bool) or not isinstance(revision, int) or not -(2**63) <= revision < 2**63:
            raise ApiError(400, Err.INVALID_PAYLOAD, "revision must be a signed 64-bit integer.")
        self._check_revision(body, party["revision"])
        if "config" in body and not isinstance(body["config"], dict):
            raise ApiError(400, Err.INVALID_PAYLOAD, "config must be an object.")
        config = self._config(body.get("config"), party["config"])
        if config["maxSize"] < len(party["members"]):
            raise ApiError(400, Err.INVALID_PAYLOAD, "max_size cannot be less than the current member count.")
        if "meta" in body and not isinstance(body["meta"], dict):
            raise ApiError(400, Err.INVALID_PAYLOAD, "meta must be an object.")
        meta_body = body.get("meta", {})
        deletes, updates = self._meta_body(meta_body)
        override = meta_body.get("override", {})
        if not isinstance(override, dict) or not all(isinstance(key, str) for key in override):
            raise ApiError(400, Err.INVALID_PAYLOAD, "override must be an object.")
        # Local policy: merge override, then update, after deletions; server semantics are unverified.
        meta = copy.deepcopy(party["meta"])
        for key in deletes:
            meta.pop(key, None)
        meta.update(copy.deepcopy(override))
        meta.update(copy.deepcopy(updates))
        party["config"] = config
        party["meta"] = meta
        self._touch(party)
        self._emit(party, "PARTY_UPDATED", caller, party_state_removed=deletes,
                   party_state_updated=updates, party_state_overrode=override)
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
        decision = match.group(3).lower()
        body = req.json_object()
        if decision == "reject" and "reason" in body and not isinstance(body["reason"], str):
            raise ApiError(400, Err.INVALID_PAYLOAD, "reason must be a string.")
        pending = self._applicants.get(party_id, {})
        applicant = pending.get(account_id)
        if applicant is None:
            raise ApiError(404, Err.APPLICANT_NOT_FOUND,
                           f"No pending join request exists for {account_id}.")
        if decision == "reject":
            del pending[account_id]
            self._emit(party, "MEMBER_REJECTED", account_id, recipients=[account_id],
                       member=applicant["member"], include_connection=applicant["has_connection"],
                       reason=body.get("reason", ""))
            return 204, None
        current = self._party_of(account_id)
        if current is not None:
            raise ApiError(409, Err.USER_HAS_PARTY, "The user is already in a party.")
        if len(party["members"]) >= party["config"]["maxSize"]:
            raise ApiError(409, Err.PARTY_IS_FULL, "The party is full.")
        member = applicant["member"]
        expected_build = party["meta"].get(BUILD_KEY)
        if expected_build is not None and member["meta"].get(BUILD_KEY) != expected_build:
            raise ApiError(403, Err.PARTY_JOIN_FORBIDDEN, "The build does not match this party.")
        if party["config"]["joinability"] != "OPEN" and self._find_invite(party, account_id) is None:
            raise ApiError(403, Err.PARTY_JOIN_FORBIDDEN, "An invite is required to join this party.")
        now = self.ctx.iso()
        member["joined_at"] = member["updated_at"] = now
        for connection in member["connections"]:
            connection["connected_at"] = connection["updated_at"] = now
        self._admit(party, account_id, member)
        return 204, None

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
        self._emit(party, "MEMBER_STATE_UPDATED", caller,
                   member_state_removed=[], member_state_updated={})
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
        router.add("PATCH", rf"{P}/parties/([^/]+)", self.party_update, name="party.update", status="observed")
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
