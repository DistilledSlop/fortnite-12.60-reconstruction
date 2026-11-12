"""Local game-server profile/results bridge and inferred StatsV2 responses.

The /local protocol is ours, not Epic's. Only an authenticated dedicated server
may read admitted players' loadouts or report results. Neither client MCP nor
public stats requests can award progression.
"""

import copy

from api_common import ApiError, Err


class GameplayService:
    def __init__(self, ctx, accounts, profiles, oauth, party, gameservers, progression):
        self.ctx, self.accounts, self.profiles = ctx, accounts, profiles
        self.oauth, self.party = oauth, party
        self.gameservers, self.progression = gameservers, progression

    def _member(self, session_id, account_id, *, include_expired=False):
        session = self.gameservers.get_session(session_id, include_expired=include_expired)
        if session is None:
            raise ApiError(404, Err.NOT_FOUND, "The game-server session was not found or expired.")
        if account_id not in session["players"]:
            raise ApiError(403, Err.MISSING_PERMISSION, "The player is not admitted to this session.")
        if self.accounts.get(account_id) is None:
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, "The player account was not found.")
        return session

    def player(self, req, match):
        self.gameservers.authorize(req)
        session_id, account_id = match.group(1, 2)
        self._member(session_id, account_id)
        profile = self.profiles.profile(account_id, "athena")
        stats = profile["stats"]["attributes"]
        locker_id = stats.get("last_applied_loadout")
        locker = profile["items"].get(locker_id)
        if locker is None or not locker["templateId"].lower().startswith("cosmeticlocker:"):
            raise ApiError(409, Err.INVALID_PAYLOAD, "The active cosmetic loadout is unavailable.")
        attrs = locker["attributes"]
        slots = attrs.get("locker_slots_data", {}).get("slots", {})
        party = self.party._party_of(account_id)
        return 200, {
            "accountId": account_id, "displayName": self.accounts.display_name(account_id),
            "profileRevision": profile["rvn"], "loadoutId": locker_id,
            "cosmetics": {category: list(slot.get("items", [])) for category, slot in slots.items()},
            "variants": {category: copy.deepcopy(slot.get("activeVariants", []))
                         for category, slot in slots.items()},
            "bannerIcon": attrs.get("banner_icon_template", stats.get("banner_icon", "")),
            "bannerColor": attrs.get("banner_color_template", stats.get("banner_color", "")),
            "partyId": party["id"] if party else None,
            "partyMembers": [member["account_id"] for member in party["members"]] if party else [],
        }

    def result(self, req, match):
        self.gameservers.authorize(req)
        session_id = match.group(1)
        payload = req.json_object()
        # Exact retries must remain possible after process-local registry expiry
        # or restart. Authentication and the durable canonical payload still apply.
        replay = self.progression.replay_result(session_id, payload)
        if replay is not None:
            return 200, replay
        self._member(session_id, payload.get("accountId"), include_expired=True)
        return 200, self.progression.record_result(session_id, payload)

    @staticmethod
    def _time(req, key):
        value = req.query.get(key)
        if value is None:
            return None
        if not isinstance(value, str) or not value.isascii() or not value.isdecimal() or len(value) > 19:
            raise ApiError(400, Err.INVALID_PAYLOAD, f"{key} must be a nonnegative Unix timestamp.")
        return int(value)

    def stats(self, req, match):
        self.oauth.account_session(req)
        result = self.progression.stats(match.group(1), self._time(req, "startTime"),
                                        self._time(req, "endTime"))
        requested = req.query_all.get("stat", [])
        if requested:
            result["stats"] = {key: value for key, value in result["stats"].items() if key in requested}
        return 200, result

    def register(self, router):
        router.add("GET", r"/local/gameserver/([^/]+)/player/([^/]+)", self.player,
                   name="gameserver.player", status="guess")
        router.add("POST", r"/local/gameserver/([^/]+)/result", self.result,
                   name="gameserver.result", status="guess")
        router.add("GET", r"/statsproxy/api/statsv2/account/([^/]+)", self.stats,
                   name="stats.account", status="inferred")
