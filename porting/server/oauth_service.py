"""OAuth token, exchange, verify and session-kill endpoints (local stand-in).

Evidence:
  observed  POST /account/api/oauth/token (grants client_credentials and
            exchange_code), DELETE /account/api/oauth/sessions/kill and
            .../sessions/kill/{token}. The response field names are in the binary.
  inferred  GET /account/api/oauth/exchange and GET /account/api/oauth/verify.
            Their paths come from the route list in routes.json. The bodies are guesses.

Policy (local, deliberate):
  * Access and refresh tokens are random per process. They live only in memory.
  * An exchange_code that GET .../oauth/exchange issued is single use and maps to
    its own account. Unknown, expired and used codes are rejected.
  * The local_username grant and the local-username: exchange-code prefix select
    a persistent local account by username without a password.
  * Every token response is synthetic. No real account, client secret or key is used.
"""

import secrets
from dataclasses import dataclass
from typing import Optional

from api_common import (CLIENT_ID, DEVICE_ID, ApiError, Err)

ACCESS_TTL_SEC = 28800  # the field value from the earlier stub
REFRESH_TTL_SEC = 115200  # the field value from the earlier stub
EXCHANGE_CODE_TTL_SEC = 300  # guess
LOCAL_USERNAME_PREFIX = "local-username:"
EXCHANGE_CODE_PREFIX = "local-exchange:"


@dataclass
class Session:
    access_token: str
    refresh_token: str
    account_id: Optional[str]  # None for a client_credentials token
    client_id: str
    display_name: str
    grant: str
    session_id: str
    access_expires: float
    refresh_expires: float


class OAuthService:
    def __init__(self, ctx, accounts):
        self.ctx = ctx
        self.accounts = accounts
        self._by_access = {}
        self._by_refresh = {}
        self._codes = {}

    # Token store -------------------------------------------------------

    def _issue(self, grant, account_id, client_id):
        now = self.ctx.now()
        account = self.accounts.get(account_id) if account_id else None
        session = Session(
            access_token="local-" + secrets.token_hex(24),
            refresh_token="localr-" + secrets.token_hex(24),
            account_id=account_id,
            client_id=client_id,
            display_name=account["displayName"] if account else "",
            grant=grant,
            session_id=secrets.token_hex(16),
            access_expires=now + ACCESS_TTL_SEC,
            refresh_expires=now + REFRESH_TTL_SEC,
        )
        self._by_access[session.access_token] = session
        self._by_refresh[session.refresh_token] = session
        return session

    def _drop(self, session):
        self._by_access.pop(session.access_token, None)
        self._by_refresh.pop(session.refresh_token, None)

    def _live(self, session):
        return session is not None and session.access_expires > self.ctx.now()

    def lookup(self, token):
        session = self._by_access.get(token)
        return session if self._live(session) else None

    def authenticate(self, req, account_id=None):
        """Return the session behind the request's bearer token, or raise 401/403.

        With account_id, the token's account must match it (403 otherwise). A
        client_credentials token has no account, so it never matches an account route.
        """
        token = req.bearer_token()
        if token is None:
            raise ApiError(401, Err.AUTH_REQUIRED, "Authentication is required for this request.")
        session = self.lookup(token)
        if session is None:
            raise ApiError(401, Err.TOKEN_INVALID, "The access token is invalid or has expired.")
        if account_id is not None and session.account_id != account_id:
            raise ApiError(403, Err.MISSING_PERMISSION, "The token does not grant access to this account.")
        return session

    def account_session(self, req):
        """Authenticate and require a user token (one that has an account)."""
        session = self.authenticate(req)
        if session.account_id is None:
            raise ApiError(401, Err.AUTH_REQUIRED, "A user access token is required for this request.")
        return session

    # Responses ---------------------------------------------------------

    def _token_body(self, session):
        body = {
            "access_token": session.access_token,
            "expires_in": ACCESS_TTL_SEC,
            "expires_at": self.ctx.iso(session.access_expires),
            "token_type": "bearer",
            "refresh_token": session.refresh_token,
            "refresh_expires": REFRESH_TTL_SEC,
            "refresh_expires_at": self.ctx.iso(session.refresh_expires),
            "client_id": session.client_id,
            "internal_client": True,
            "client_service": "fortnite",
            "app": "fortnite",
            "device_id": DEVICE_ID,
        }
        if session.account_id is not None:
            body.update({
                "account_id": session.account_id,
                "displayName": session.display_name,
                "in_app_id": session.account_id,
            })
        return body

    # Handlers ----------------------------------------------------------

    def _username_account_id(self, username):
        try:
            return self.accounts.get_or_create_username(username)["id"]
        except ValueError as exc:
            raise ApiError(400, Err.OAUTH_INVALID_REQUEST, str(exc)) from exc

    def token(self, req, match):
        form = req.form
        grant = form.get("grant_type", "")
        client_id = form.get("client_id") or CLIENT_ID
        if not grant:
            raise ApiError(400, Err.OAUTH_INVALID_REQUEST, "grant_type is required.")
        if grant == "client_credentials":
            session = self._issue(grant, None, client_id)
        elif grant == "local_username":
            account_id = self._username_account_id(form.get("username", ""))
            session = self._issue(grant, account_id, client_id)
        elif grant == "exchange_code":
            code = form.get("exchange_code", "")
            if not code:
                raise ApiError(400, Err.OAUTH_INVALID_REQUEST, "exchange_code is required.")
            issued = self._codes.pop(code, None)
            if issued is not None:
                account_id, expires = issued
                if expires <= self.ctx.now():
                    raise ApiError(400, Err.OAUTH_INVALID_GRANT, "The exchange code has expired.")
            elif code.startswith(EXCHANGE_CODE_PREFIX):
                raise ApiError(400, Err.OAUTH_INVALID_GRANT, "The exchange code is invalid or has expired.")
            elif code.startswith(LOCAL_USERNAME_PREFIX):
                account_id = self._username_account_id(code[len(LOCAL_USERNAME_PREFIX):])
            else:
                raise ApiError(400, Err.OAUTH_INVALID_GRANT, "The exchange code is invalid or has expired.")
            session = self._issue(grant, account_id, client_id)
        elif grant == "refresh_token":
            old = self._by_refresh.get(form.get("refresh_token", ""))
            if old is None or old.refresh_expires <= self.ctx.now():
                raise ApiError(400, Err.REFRESH_INVALID, "The refresh token is invalid.")
            self._drop(old)
            session = self._issue(grant, old.account_id, old.client_id)
        else:
            raise ApiError(400, Err.OAUTH_UNSUPPORTED_GRANT, f"The grant type {grant} is not supported locally.")
        return 200, self._token_body(session)

    def exchange(self, req, match):
        """GET /account/api/oauth/exchange: issue a single-use code for the caller's account."""
        session = self.account_session(req)
        code = EXCHANGE_CODE_PREFIX + secrets.token_hex(16)
        self._codes[code] = (session.account_id, self.ctx.now() + EXCHANGE_CODE_TTL_SEC)
        return 200, {
            "expiresInSeconds": EXCHANGE_CODE_TTL_SEC,
            "code": code,
            "creatingClientId": session.client_id,
        }

    def verify(self, req, match):
        session = self.authenticate(req)
        remaining = max(0, int(session.access_expires - self.ctx.now()))
        body = {
            "session_id": session.session_id,
            "token_type": "bearer",
            "client_id": session.client_id,
            "internal_client": True,
            "client_service": "fortnite",
            "expires_in": remaining,
            "expires_at": self.ctx.iso(session.access_expires),
            "auth_method": session.grant,
            "app": "fortnite",
            "device_id": DEVICE_ID,
        }
        if session.account_id is not None:
            body.update({
                "account_id": session.account_id,
                "display_name": session.display_name,
                "in_app_id": session.account_id,
            })
        return 200, body

    def local_sign_in(self, account_id):
        """Operator and test hook, not an HTTP route: issue a token body for an account."""
        if self.accounts.get(account_id) is None:
            raise ValueError(f"unknown account {account_id}")
        return self._token_body(self._issue("local_sign_in", account_id, CLIENT_ID))

    def kill_all(self, req, match):
        """DELETE /account/api/oauth/sessions/kill: end every session of the caller's account."""
        session = self.authenticate(req)
        for other in list(self._by_access.values()):
            if other is session or (session.account_id is not None and other.account_id == session.account_id):
                self._drop(other)
        return 204, None

    def kill_one(self, req, match):
        """DELETE .../sessions/kill/{token}: end one session. Only the owner's sessions can be removed."""
        session = self.authenticate(req)
        target = self._by_access.get(match.group(1))
        if target is None:
            return 204, None
        if target is not session and (session.account_id is None or target.account_id != session.account_id):
            raise ApiError(403, Err.NOT_OWN_SESSION, "A session that belongs to another account cannot be removed.")
        self._drop(target)
        return 204, None

    def register(self, router):
        router.add("POST", r"/account/api/oauth/token", self.token, name="oauth.token", status="observed")
        router.add("GET", r"/account/api/oauth/exchange", self.exchange, name="oauth.exchange", status="inferred")
        router.add("GET", r"/account/api/oauth/verify", self.verify, name="oauth.verify", status="inferred")
        router.add("DELETE", r"/account/api/oauth/sessions/kill", self.kill_all,
                   name="oauth.sessions.kill", status="observed")
        router.add("DELETE", r"/account/api/oauth/sessions/kill/([^/]+)", self.kill_one,
                   name="oauth.sessions.kill_token", status="observed")
