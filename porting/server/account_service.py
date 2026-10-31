"""Account lookup and account-level game endpoints (local stand-in).

Evidence:
  observed  GET /account/api/public/account/{id}, .../account?accountId=,
            .../{id}/externalAuths, POST /api/v1/user/setting,
            POST /fortnite/api/game/v2/tryPlayOnPlatform/account/{id},
            POST /fortnite/api/game/v2/profileToken/verify/{id}. These come from the
            earlier login capture in porting/server_surface.md.
  inferred  GET /account/api/public/account/displayName/{name}. The path template is
            in the binary (/account/displayName/`name). The response shape is not.

Synthetic accounts only. The default account is LocalPlayer, with id ...0001. Unknown
ids now answer 404. The earlier stub answered every id with LocalPlayer.
"""

from api_common import (LOCAL_ACCOUNT_ID, LOCAL_DISPLAY_NAME, ApiError, Err, RawText)


class AccountService:
    def __init__(self, ctx):
        self.ctx = ctx
        self._by_id = {}
        self._by_name = {}
        self._next = 2
        self.create(LOCAL_DISPLAY_NAME, account_id=LOCAL_ACCOUNT_ID)

    def create(self, display_name, account_id=None):
        """Add a synthetic account. Ids are sequential (0...02, 0...03, ...) unless given."""
        if account_id is None:
            account_id = f"{self._next:032x}"
            self._next += 1
        if account_id in self._by_id:
            raise ValueError(f"account {account_id} already exists")
        if display_name.lower() in self._by_name:
            raise ValueError(f"display name {display_name} is already used")
        account = {
            "id": account_id,
            "displayName": display_name,
            "name": "Local",
            "email": f"{account_id}@localhost.invalid",
            "failedLoginAttempts": 0,
            "lastLogin": self.ctx.iso(),
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
        self._by_id[account_id] = account
        self._by_name[display_name.lower()] = account_id
        return dict(account)

    def get(self, account_id):
        account = self._by_id.get(account_id)
        return dict(account) if account else None

    def display_name(self, account_id):
        account = self._by_id.get(account_id)
        return account["displayName"] if account else LOCAL_DISPLAY_NAME

    # Handlers ----------------------------------------------------------

    def account_by_id(self, req, match):
        account = self.get(match.group(1))
        if account is None:
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, "Sorry, we couldn't find an account for that id.")
        return 200, account

    def accounts_by_ids(self, req, match):
        ids = req.query_all.get("accountId", [])
        if not ids or len(ids) > 100:
            raise ApiError(400, Err.INVALID_ACCOUNT_COUNT, "Between 1 and 100 accountId values are required.")
        found = [self.get(account_id) for account_id in ids]
        return 200, [account for account in found if account is not None]

    def account_by_name(self, req, match):
        account_id = self._by_name.get(match.group(1).lower())
        account = self.get(account_id) if account_id else None
        if account is None:
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, "Sorry, we couldn't find an account for that display name.")
        return 200, account

    def external_auths(self, req, match):
        if self.get(match.group(1)) is None:
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, "Sorry, we couldn't find an account for that id.")
        return 200, []

    def user_setting(self, req, match):
        return 200, []

    def try_play_on_platform(self, req, match):
        # Observed body: text "true". The platform query value is not checked locally.
        return 200, RawText("true")

    def verify_profile_token(self, req, match):
        if self.get(match.group(1)) is None:
            raise ApiError(404, Err.ACCOUNT_NOT_FOUND, "Sorry, we couldn't find an account for that id.")
        return 204, None

    def register(self, router):
        router.add("GET", r"/account/api/public/account/([^/]+)", self.account_by_id,
                   name="account.by_id", status="observed")
        router.add("GET", r"/account/api/public/account", self.accounts_by_ids,
                   name="account.by_ids", status="observed")
        router.add("GET", r"/account/api/public/account/displayName/([^/]+)", self.account_by_name,
                   name="account.by_display_name", status="inferred")
        router.add("GET", r"/account/api/public/account/([^/]+)/externalAuths", self.external_auths,
                   name="account.external_auths", status="observed")
        router.add("POST", r"/api/v1/user/setting", self.user_setting,
                   name="account.user_setting", status="observed")
        router.add("POST", r"/fortnite/api/game/v2/tryPlayOnPlatform/account/[^/]+", self.try_play_on_platform,
                   name="account.try_play_on_platform", status="observed")
        router.add("POST", r"/fortnite/api/game/v2/profileToken/verify/([^/]+)", self.verify_profile_token,
                   name="account.profile_token_verify", status="observed")
