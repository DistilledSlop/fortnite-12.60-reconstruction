"""Minimal plausible Epic service responses so the original client advances.

Each handler gets (method, path, query dict, form dict) and returns (status, body) or None.
Tokens and ids are fixed, non-secret placeholders for the local backend only.
"""
import re

ACCOUNT_ID = "00000000000000000000000000000001"
DISPLAY_NAME = "LocalPlayer"
FAR_FUTURE = "2099-01-01T00:00:00.000Z"


class RawText(str):
    """Marks a response sent as text/plain rather than JSON."""


def not_found(path):
    return 404, {
        "errorCode": "errors.com.epicgames.common.not_found",
        "errorMessage": f"Sorry, the resource {path} was not found",
        "numericErrorCode": 1004,
        "originatingService": "local",
        "intent": "prod",
    }


def oauth_token(form):
    return 200, {
        "access_token": "local" + "a" * 27,
        "expires_in": 28800,
        "expires_at": FAR_FUTURE,
        "token_type": "bearer",
        "refresh_token": "local" + "r" * 27,
        "refresh_expires": 115200,
        "refresh_expires_at": FAR_FUTURE,
        "account_id": ACCOUNT_ID,
        "client_id": "3446cd72694c4a4485d81b77adbb2141",
        "internal_client": True,
        "client_service": "fortnite",
        "displayName": DISPLAY_NAME,
        "app": "fortnite",
        "in_app_id": ACCOUNT_ID,
        "device_id": "0" * 32,
    }


def account(path_id):
    return 200, {
        "id": path_id,
        "displayName": DISPLAY_NAME,
        "name": "Local",
        "email": "local@localhost",
        "failedLoginAttempts": 0,
        "lastLogin": "2026-01-01T00:00:00.000Z",
        "numberOfDisplayNameChanges": 0,
        "ageGroup": "ADULT",
        "headless": False,
        "country": "US",
        "lastName": "Player",
        "preferredLanguage": "en",
        "canUpdateDisplayName": False,
        "tfaEnabled": False,
        "emailVerified": True,
        "minorVerified": False,
        "minorExpected": False,
        "minorStatus": "NOT_MINOR",
    }


def lightswitch():
    return 200, [{
        "serviceInstanceId": "fortnite",
        "status": "UP",
        "message": "Fortnite is online",
        "maintenanceUri": None,
        "overrideCatalogIds": ["a7f138b2e51945ffbfdacc1af0541053"],
        "allowedActions": ["PLAY", "DOWNLOAD"],
        "banned": False,
        "launcherInfoDTO": {"appName": "Fortnite", "catalogItemId": "4fe75bbc5a674f4f9b356b5c90567da5", "namespace": "fn"},
    }]


def calendar():
    state = {"activeStorefronts": [], "eventNamedWeights": {}, "seasonNumber": 12,
             "seasonTemplateId": "AthenaSeason:athenaseason12", "matchXpBonusPoints": 0,
             "seasonBegin": "2020-06-02T00:00:00Z", "seasonEnd": FAR_FUTURE, "seasonDisplayedEnd": FAR_FUTURE,
             "weeklyStoreEnd": FAR_FUTURE, "stwEventStoreEnd": FAR_FUTURE,
             "stwWeeklyStoreEnd": FAR_FUTURE, "dailyStoreEnd": FAR_FUTURE}
    return {"channels": {
        "client-matchmaking": {"states": [], "cacheExpire": FAR_FUTURE},
        "client-events": {"states": [{"validFrom": "2020-01-01T00:00:00.000Z", "activeEvents": [], "state": state}],
                          "cacheExpire": FAR_FUTURE}},
        "eventsTimeOffsetHrs": 0.0, "cacheIntervalMins": 10.0, "currentTime": "2026-01-01T00:00:00.000Z"}


def profile(account_id, query, command):
    """Minimal fullProfileUpdate for any profileId; the client reads items and stats.attributes."""
    profile_id = query.get("profileId", "common_core")
    now = "2026-01-01T00:00:00.000Z"
    attributes = {"season_num": 12, "level": 1, "book_level": 1, "xp": 0, "accountLevel": 1}
    if profile_id == "common_core":
        attributes = {"mfa_reward_claimed": True, "current_mtx_platform": "EpicPC", "mtx_affiliate": "",
                      "survey_data": {}, "personal_offers": {}, "intro_game_played": True,
                      "import_friends_claimed": {}, "allowed_to_send_gifts": True,
                      "allowed_to_receive_gifts": True, "gift_history": {}}
    elif profile_id == "common_public":
        attributes = {"banner_color": "DefaultColor1", "banner_icon": "StandardBanner1",
                      "homebase_name": DISPLAY_NAME}
    rvn = 1
    return 200, {
        "profileRevision": rvn,
        "profileId": profile_id,
        "profileChangesBaseRevision": rvn,
        "profileChanges": [{"changeType": "fullProfileUpdate", "profile": {
            "_id": account_id, "created": now, "updated": now, "rvn": rvn, "wipeNumber": 1,
            "accountId": account_id, "profileId": profile_id, "version": "no_version",
            "items": {}, "stats": {"attributes": attributes}, "commandRevision": rvn}}],
        "profileCommandRevision": rvn,
        "serverTime": now,
        "responseVersion": 1,
    }


ROUTES = [
    ("GET", r"/fortnite/api/receipts/v1/account/[^/]+/receipts", lambda m, q, f: (200, [])),
    ("GET", r"/socialban/api/public/v1/[^/]+", lambda m, q, f: (200, {"bans": [], "warnings": []})),
    ("GET", r"/party/api/v1/Fortnite/user/[^/]+", lambda m, q, f: (200, {"current": [], "pending": [], "invites": [], "pings": []})),
    ("GET", r"/party/api/v1/Fortnite/user/[^/]+/notifications/undelivered/count", lambda m, q, f: (200, {"pings": 0, "invites": 0})),
    ("GET", r"/fortnite/api/calendar/v1/timeline", lambda m, q, f: (200, calendar())),
    ("GET", r"/fortnite/api/storefront/v2/catalog", lambda m, q, f: (200, {
        "refreshIntervalHrs": 1, "dailyPurchaseHrs": 24, "expiration": FAR_FUTURE, "storefronts": []})),
    ("GET", r"/presence/api/v1/_/[^/]+/(last-online|subscriptions)", lambda m, q, f: (200, {})),
    ("GET", r"/presence/api/v1/Fortnite/[^/]+/subscriptions/nudged", lambda m, q, f: (200, [])),
    ("GET", r"/api/v1/Fortnite/get", lambda m, q, f: (200, [])),
    ("PUT", r"/fortnite/api/cloudstorage/user/[^/]+/[^/]+", lambda m, q, f: (204, None)),
    ("GET", r"/friends/api/v1/[^/]+/summary", lambda m, q, f: (200, {
        "friends": [], "incoming": [], "outgoing": [], "suggested": [], "blocklist": [],
        "settings": {"acceptInvites": "public"},
        "limitsReached": {"incoming": False, "outgoing": False, "accepted": False}})),
    ("GET", r"/friends/api/v1/[^/]+/(blocklist|recent/[^/]+)", lambda m, q, f: (200, [])),
    ("GET", r"/friends/api/v1/[^/]+/settings", lambda m, q, f: (200, {"acceptInvites": "public"})),
    ("GET", r"/presence/api/v1/_/[^/]+/settings/subscriptions", lambda m, q, f: (200, [])),
    ("GET", r"/fortnite/api/game/v2/privacy/account/[^/]+", lambda m, q, f: (200, {"acceptInvites": "public"})),
    ("GET", r"/content/api/pages/fortnite-game", lambda m, q, f: (200, {"_title": "Fortnite Game", "_activeDate": "2026-01-01T00:00:00.000Z", "lastModified": "2026-01-01T00:00:00.000Z", "_locale": "en-US"})),
    ("GET", r"/fortnite/api/storefront/v2/keychain", lambda m, q, f: (200, ["00000000000000000000000000000000:AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=:/Game/Local/Unused"])),
    ("POST", r"/fortnite/api/game/v2/profile/([^/]+)/(client|dedicated_server)/([A-Za-z]+)", lambda m, q, f: profile(m.group(1), q, m.group(3))),
    ("GET", r"/fortnite/api/cloudstorage/user/[^/]+", lambda m, q, f: (200, [])),
    ("GET", r"/fortnite/api/game/v2/enabled_features", lambda m, q, f: (200, [])),
    ("POST", r"/fortnite/api/storeaccess/v1/request_access/[^/]+", lambda m, q, f: (204, None)),
    ("GET", r"/lightswitch/api/service/(bulk/status|[^/]+/status)", lambda m, q, f: lightswitch()),
    ("GET", r"/account/api/public/account", lambda m, q, f: (200, [account(q.get("accountId", ACCOUNT_ID))[1]])),
    ("POST", r"/api/v1/user/setting", lambda m, q, f: (200, [])),
    ("POST", r"/fortnite/api/game/v2/tryPlayOnPlatform/account/[^/]+", lambda m, q, f: (200, RawText("true"))),
    ("POST", r"/fortnite/api/game/v2/profileToken/verify/[^/]+", lambda m, q, f: (204, None)),
    ("GET", r"/account/api/public/account/([^/]+)", lambda m, q, f: account(m.group(1))),
    ("GET", r"/account/api/public/account/[^/]+/externalAuths", lambda m, q, f: (200, [])),
    ("DELETE", r"/account/api/oauth/sessions/kill(/[^/]+)?", lambda m, q, f: (204, None)),
    ("GET", r"/fortnite/api/v2/versioncheck/[^/]+", lambda m, q, f: (200, {"type": "NO_UPDATE"})),
    ("POST", r"/account/api/oauth/token", lambda m, q, f: oauth_token(f)),
    ("GET", r"/fortnite/api/cloudstorage/system", lambda m, q, f: (200, [])),
    ("POST", r"/datarouter/api/v1/public/data", lambda m, q, f: (204, None)),
]


def respond(method, path, query, form):
    for route_method, pattern, handler in ROUTES:
        match = re.fullmatch(pattern, path, re.I)
        if route_method == method and match:
            return handler(match, query, form)
    return not_found(path)
