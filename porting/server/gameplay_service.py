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

    def quest_event(self, req, match):
        self.gameservers.authorize(req)
        session_id = match.group(1)
        payload = req.json_object()
        if set(payload) - {'accountId', 'eventId', 'event', 'amount'}:
            raise ApiError(400, Err.INVALID_PAYLOAD, 'Invalid quest event payload.')
        event_id = payload.get('eventId')
        if not isinstance(event_id, str) or not event_id or len(event_id) > 128:
            raise ApiError(400, Err.INVALID_PAYLOAD, 'Invalid quest event ID.')
        receipt_id = 'gameserver:' + session_id + ':' + event_id
        account_id, event, amount = payload.get('accountId'), payload.get('event'), payload.get('amount', 1)
        quests = self.profiles.quests
        replay = quests.replay_progress(account_id, event, amount, event_id=receipt_id, match_id=session_id)
        if replay is not None:
            return 200, {'accepted': replay}
        self._member(session_id, account_id)
        return 200, {'accepted': quests.progress(account_id, event, amount, event_id=receipt_id, match_id=session_id)}

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

    def stats_query(self, req, match):
        """Read a StatsV2 batch, using the same local visibility policy as GET.

        Original client captures under /opt/fortnite/local-model-package/captures:
        0bd58650353647638bbb3cf02fd15fc1/0x140bb28a0.c (POST and validation),
        f40086b73f404a3ea8f04c1015e0faeb/0x140bb35d0.c (array response),
        da2af36046ad440595e934fbe48ebec8/0x140bca120.c (result fields),
        ff1d260f787c44739dd130fe2b0eeb0f/0x140bc9c20.c (request fields).
        Server limits, appId policy, unknown owners and time semantics are
        local policies rather than original-server proof.
        Locally, bounds are inclusive Unix seconds, duplicate owners collapse
        in first-seen order, and an unknown owner fails the entire batch.
        """
        self.oauth.account_session(req)
        body = req.json_object()
        def invalid(message):
            raise ApiError(400, Err.INVALID_PAYLOAD, message)

        app_id = body.get("appId")
        if not isinstance(app_id, str) or not app_id.strip() or len(app_id) > 255:
            invalid("appId must be a nonempty string of at most 255 characters.")
        arrays = []
        for field in ("owners", "stats"):
            values = body.get(field)
            if (not isinstance(values, list) or not 1 <= len(values) <= 100 or
                    any(not isinstance(value, str) or not value.strip() or len(value) > 255
                        for value in values)):
                invalid(f"{field} must contain 1 to 100 nonempty strings of at most 255 characters.")
            arrays.append(values)
        start, end = body.get("startDate"), body.get("endDate")
        if (type(start) is not int or type(end) is not int or
                not 0 <= start <= end <= 2**63 - 1):
            invalid("startDate and endDate must be ordered nonnegative int64 Unix timestamps.")
        owners, names = arrays
        requested = set(names)
        results = []
        for owner in dict.fromkeys(owners):
            result = self.progression.stats(owner, start, end)
            result["stats"] = {key: value for key, value in result["stats"].items() if key in requested}
            results.append(result)
        return 200, results

    def register(self, router):
        router.add("GET", r"/local/gameserver/([^/]+)/player/([^/]+)", self.player,
                   name="gameserver.player", status="guess")
        router.add("POST", r"/local/gameserver/([^/]+)/result", self.result,
                   name="gameserver.result", status="guess")
        router.add("POST", r"/local/gameserver/([^/]+)/quest-event", self.quest_event,
                   name="gameserver.quest_event", status="guess")
        router.add("GET", r"/statsproxy/api/statsv2/account/([^/]+)", self.stats,
                   name="stats.account", status="inferred")
        router.add("POST", r"/statsproxy/api/statsv2/query", self.stats_query,
                   name="stats.query", status="inferred")
