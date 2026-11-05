"""Shared types for the local Epic service stand-ins (Python standard library only).

Every route carries an evidence label. backend.py --routes prints the table.

  observed  the name or path appears in the 12.60 Win64 client binary strings,
            in the DefaultEngine.ini extract, or in a login capture recorded in
            porting/server_surface.md.
  inferred  the path is observed, but the method, body or response shape comes
            from public Epic API documentation or is reasoned from nearby
            evidence. Not verified against a client capture.
  guess     invented for this local stand-in, or kept from the earlier stub so
            the client keeps moving. Unverified.

Nothing here is a credential. Tokens and session keys are generated with
`secrets` at run time, kept in memory and never written to a log.
"""

import hashlib
import itertools
import json
import re
import time
from datetime import datetime, timezone

DEFAULT_SEED = "fortnite-12.60-local-backend"
LOCAL_ACCOUNT_ID = "00000000000000000000000000000001"
LOCAL_DISPLAY_NAME = "LocalPlayer"
# guess: client id carried over from the earlier stub. Not found in the 12.60 binary strings.
CLIENT_ID = "3446cd72694c4a4485d81b77adbb2141"
# guess: placeholder device id (32 zeros), as in the earlier stub.
DEVICE_ID = "0" * 32


def log_safe_path(path):
    """Remove the bearer token carried in the OAuth session-kill URL."""
    return re.sub(r"(?i)(/account/api/oauth/sessions/kill/).*", r"\1<redacted>", path)


class Err:
    """Error codes. The comment says whether the code string appears in the 12.60 binary."""

    NOT_FOUND = "errors.com.epicgames.not_found"  # observed
    METHOD_NOT_ALLOWED = "errors.com.epicgames.common.method_not_allowed"  # observed
    JSON_PARSE = "errors.com.epicgames.common.json_parse_error"  # observed
    AUTH_REQUIRED = "errors.com.epicgames.common.oauth_authentication_required"  # observed
    TOKEN_INVALID = "errors.com.epicgames.account.auth_token.invalid_access_token"  # observed
    MISSING_PERMISSION = "errors.com.epicgames.common.missing_permission"  # observed
    OAUTH_INVALID_REQUEST = "errors.com.epicgames.common.oauth.invalid_request"  # observed
    OAUTH_INVALID_GRANT = "errors.com.epicgames.common.oauth.invalid_grant"  # observed
    OAUTH_UNSUPPORTED_GRANT = "errors.com.epicgames.common.oauth.unsupported_grant_type"  # observed
    REFRESH_INVALID = "errors.com.epicgames.account.auth_token.invalid_refresh_token"  # observed
    NOT_OWN_SESSION = "errors.com.epicgames.account.auth_token.not_own_session_removal"  # observed
    ACCOUNT_NOT_FOUND = "errors.com.epicgames.account.account_not_found"  # observed
    INVALID_ACCOUNT_COUNT = "errors.com.epicgames.account.invalid_account_id_count"  # observed
    PROFILE_ACCOUNT_NOT_FOUND = "errors.com.epicgames.modules.profile.account_not_found"  # observed
    PROFILE_NOT_FOUND = "errors.com.epicgames.modules.profile.profile_not_found"  # observed
    PROFILE_ID_INVALID = "errors.com.epicgames.modules.profile.invalid_profile_id_param"  # observed
    OPERATION_NOT_FOUND = "errors.com.epicgames.modules.profile.operation_not_found"  # observed
    INVALID_PROFILE_COMMAND = "errors.com.epicgames.modules.profile.invalid_profile_command"  # observed
    INVALID_PAYLOAD = "errors.com.epicgames.modules.profile.invalid_payload"  # observed
    TEMPLATE_NOT_FOUND = "errors.com.epicgames.modules.profile.template_not_found"  # observed
    CLOUD_FILE_NOT_FOUND = "errors.com.epicgames.cloudstorage.file_not_found"  # observed
    PARTY_NOT_FOUND = "errors.com.epicgames.social.party.party_not_found"  # observed
    PARTY_MEMBER_NOT_FOUND = "errors.com.epicgames.social.party.member_not_found"  # observed
    PARTY_IS_FULL = "errors.com.epicgames.social.party.party_is_full"  # observed
    PARTY_JOIN_FORBIDDEN = "errors.com.epicgames.social.party.party_join_forbidden"  # observed
    PARTY_CHANGE_FORBIDDEN = "errors.com.epicgames.social.party.party_change_forbidden"  # observed
    USER_HAS_PARTY = "errors.com.epicgames.social.party.user_has_party"  # observed
    USER_ALREADY_IN_PARTY = "errors.com.epicgames.social.party.user_already_in_party"  # observed
    USER_OPERATION_FORBIDDEN = "errors.com.epicgames.social.party.user_operation_forbidden"  # observed
    MEMBER_STATE_FORBIDDEN = "errors.com.epicgames.social.party.member_state_change_forbidden"  # observed
    INVITE_FORBIDDEN = "errors.com.epicgames.social.party.invite_forbidden"  # observed
    INVITE_ALREADY_EXISTS = "errors.com.epicgames.social.party.invite_already_exists"  # observed
    INVITE_NOT_FOUND = "errors.com.epicgames.social.party.invite_not_found"  # observed
    STALE_REVISION = "errors.com.epicgames.social.party.stale_revision"  # observed
    SELF_PING = "errors.com.epicgames.social.party.self_ping"  # observed
    APPLICANT_NOT_FOUND = "errors.com.epicgames.social.party.applicant_not_found"  # observed


class ApiError(Exception):
    """Raised by a handler. The dispatcher turns it into an HTTP error body."""

    def __init__(self, status, code, message, numeric=None):
        super().__init__(message)
        self.status = status
        self.code = code
        self.message = message
        self.numeric = numeric

    def body(self):
        return error_body(self.code, self.message, self.numeric)


def error_body(code, message, numeric=None):
    body = {
        "errorCode": code,
        "errorMessage": message,
        "messageVars": [],
        "originatingService": "local",
        "intent": "prod",
    }
    if numeric is not None:
        body["numericErrorCode"] = numeric
    return body


class RawText(str):
    """A text/plain body. Used where the client expects plain text, such as tryPlayOnPlatform."""


class Raw:
    """A body sent as-is with its own content type, such as a cloud storage file."""

    __slots__ = ("data", "content_type")

    def __init__(self, data, content_type="application/octet-stream"):
        self.data = bytes(data)
        self.content_type = content_type


class Context:
    """Clock and deterministic id source shared by every service.

    Ids come from a seed and a call counter, so two backends with the same seed and
    the same call order produce the same ids. Random secrets never come from here.
    """

    def __init__(self, clock=None, seed=DEFAULT_SEED):
        self.clock = clock or time.time
        self.seed = seed
        self._counter = itertools.count(1)

    def now(self):
        return float(self.clock())

    def iso(self, ts=None):
        ts = self.now() if ts is None else float(ts)
        moment = datetime.fromtimestamp(ts, timezone.utc)
        return moment.strftime("%Y-%m-%dT%H:%M:%S") + f".{moment.microsecond // 1000:03d}Z"

    def day(self):
        return datetime.fromtimestamp(self.now(), timezone.utc).strftime("%Y-%m-%d")

    def new_id(self, kind):
        """A 32-character hex id from the next counter value."""
        n = next(self._counter)
        return hashlib.sha256(f"{self.seed}|{kind}|{n}".encode("utf-8")).hexdigest()[:32]

    def stable_id(self, key):
        """A 32-character hex id that depends only on the seed and the key."""
        return hashlib.sha256(f"{self.seed}|stable|{key}".encode("utf-8")).hexdigest()[:32]


class Request:
    """One HTTP request as the handlers see it. Header names are lower-cased."""

    def __init__(self, method, path, query=None, query_all=None, form=None, body=b"", headers=None):
        self.method = method.upper()
        self.path = path
        self.query = dict(query or {})
        if query_all is None:
            query_all = {key: [value] for key, value in self.query.items()}
        self.query_all = {key: list(values) for key, values in query_all.items()}
        self.form = dict(form or {})
        self.body = body or b""
        self.headers = {str(key).lower(): value for key, value in (headers or {}).items()}

    def json(self, default=None):
        """Parse the body as JSON. An empty body returns `default`."""
        if not self.body:
            return default
        try:
            return json.loads(self.body.decode("utf-8"))
        except (UnicodeDecodeError, ValueError):
            raise ApiError(400, Err.JSON_PARSE, "The request body is not valid JSON.")

    def json_object(self):
        """Parse the body as a JSON object. An empty body is an empty object."""
        value = self.json(default={})
        if not isinstance(value, dict):
            raise ApiError(400, Err.INVALID_PAYLOAD, "The request body must be a JSON object.")
        return value

    def bearer_token(self):
        parts = self.headers.get("authorization", "").split(None, 1)
        if len(parts) == 2 and parts[0].lower() == "bearer" and parts[1].strip():
            return parts[1].strip()
        return None


class Route:
    __slots__ = ("method", "regex", "handler", "name", "status")

    def __init__(self, method, pattern, handler, name, status):
        self.method = method
        self.regex = re.compile(pattern, re.IGNORECASE)
        self.handler = handler
        self.name = name
        self.status = status


class Router:
    """Ordered regex routes. The first route whose method and path both match wins."""

    def __init__(self):
        self.routes = []

    def add(self, method, pattern, handler, *, name, status="inferred"):
        if status not in ("observed", "inferred", "guess"):
            raise ValueError(f"unknown evidence label {status!r}")
        self.routes.append(Route(method, pattern, handler, name, status))

    def resolve(self, method, path):
        """Return (route, match, allowed_methods). route is None when nothing matches."""
        allowed = []
        for route in self.routes:
            match = route.regex.fullmatch(path)
            if match is None:
                continue
            if route.method == method:
                return route, match, []
            allowed.append(route.method)
        return None, None, sorted(set(allowed))
