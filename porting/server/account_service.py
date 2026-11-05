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

import json
import sqlite3
from contextlib import closing
from pathlib import Path

from api_common import (LOCAL_ACCOUNT_ID, LOCAL_DISPLAY_NAME, ApiError, Err, RawText)


class AccountService:
    def __init__(self, ctx, state_dir=None):
        self.ctx = ctx
        self._by_id = {}
        self._by_name = {}
        self._next = 2
        self._database = None
        if state_dir is not None:
            directory = Path(state_dir)
            directory.mkdir(parents=True, exist_ok=True)
            self._database = directory / "accounts.sqlite3"
            with closing(sqlite3.connect(self._database)) as db, db:
                db.execute("CREATE TABLE IF NOT EXISTS accounts ("
                           "account_id TEXT PRIMARY KEY, name_key TEXT NOT NULL UNIQUE, data TEXT NOT NULL)")
                for account_id, name_key, data in db.execute("SELECT account_id, name_key, data FROM accounts"):
                    self._by_id[account_id] = json.loads(data)
                    self._by_name[name_key] = account_id
        if LOCAL_ACCOUNT_ID not in self._by_id:
            self.create(LOCAL_DISPLAY_NAME, account_id=LOCAL_ACCOUNT_ID)

    @staticmethod
    def normalize_username(username):
        """Return the chosen display name, rejecting empty or unprintable names."""
        if not isinstance(username, str):
            raise ValueError("Username must be a string.")
        username = username.strip()
        if not username or len(username) > 64 or not username.isprintable():
            raise ValueError("Username must contain 1 to 64 printable characters.")
        return username

    def get_or_create_username(self, username):
        """Open the local account for a username, creating it on first use."""
        name = self.normalize_username(username)
        account_id = self._by_name.get(name.casefold())
        return self.get(account_id) if account_id is not None else self.create(name)

    def create(self, display_name, account_id=None):
        """Add a synthetic account. Ids are sequential (0...02, 0...03, ...) unless given."""
        display_name = self.normalize_username(display_name)
        if account_id is None:
            while f"{self._next:032x}" in self._by_id:
                self._next += 1
            account_id = f"{self._next:032x}"
            self._next += 1
        if account_id in self._by_id:
            raise ValueError(f"account {account_id} already exists")
        if display_name.casefold() in self._by_name:
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
        if self._database is not None:
            with closing(sqlite3.connect(self._database)) as db, db:
                db.execute("INSERT INTO accounts (account_id, name_key, data) VALUES (?, ?, ?)",
                           (account_id, display_name.casefold(), json.dumps(account, separators=(",", ":"))))
        self._by_id[account_id] = account
        self._by_name[display_name.casefold()] = account_id
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
        account_id = self._by_name.get(match.group(1).casefold())
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
