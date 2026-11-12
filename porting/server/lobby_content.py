"""Local CMS page and optional supplied CMS snapshot.

The default is locally authored, not recovered Season 12 news. The section and
field names are based on client strings; the complete JSON shape remains a
compatibility hypothesis until exercised by the original client. A supplied
snapshot is returned as supplied, except for explicit local-asset: references.
"""

import copy
import json
import re
from pathlib import Path
from urllib.parse import quote, urlsplit

from api_common import ApiError, Err, Raw

_LOCALE = re.compile(r"[A-Za-z]{2,3}(?:-[A-Za-z0-9]{2,8})*")
_ASSET = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]*\.(?:png|jpe?g)", re.IGNORECASE)
_PREFIX = "local-asset:"


class LobbyContent:
    def __init__(self, *, content_dir=None, content_file=None, public_origin=None):
        if content_dir is not None and content_file is not None:
            raise ValueError("content_dir and content_file are mutually exclusive")
        self.origin = self._origin(public_origin)
        self.directory = None
        self.snapshot = None
        self.provenance = "local-authored"
        if content_dir is not None or content_file is not None:
            path = Path(content_file) if content_file is not None else Path(content_dir) / "content.json"
            self.directory = path.parent.resolve()
            try:
                with path.open(encoding="utf-8") as source:
                    self.snapshot = json.load(source)
            except (OSError, ValueError) as error:
                raise ValueError(f"cannot load CMS JSON {path}: {error}") from error
            if not isinstance(self.snapshot, dict):
                raise ValueError("CMS JSON must be an object")
            self.provenance = "supplied-snapshot"
            # Validate references at startup, rather than silently serving broken news.
            self._resolve(self.snapshot)

    @staticmethod
    def _origin(value):
        if value is None:
            return None
        parts = urlsplit(value)
        try:
            valid = (parts.scheme in ("http", "https") and parts.hostname and
                     parts.port != 0 and not parts.username and not parts.password and
                     parts.path in ("", "/") and not parts.query and not parts.fragment)
        except ValueError:
            valid = False
        if not valid:
            raise ValueError("public_origin must be an HTTP(S) origin without a path or credentials")
        return value.rstrip("/")

    def _asset_path(self, name):
        if self.directory is None or not _ASSET.fullmatch(name):
            raise ValueError("invalid local CMS asset name")
        path = self.directory / name
        if path.resolve().parent != self.directory or not path.is_file():
            raise ValueError("local CMS asset does not exist or escapes content directory")
        return path

    def _resolve(self, value):
        if isinstance(value, dict):
            return {key: self._resolve(item) for key, item in value.items()}
        if isinstance(value, list):
            return [self._resolve(item) for item in value]
        if isinstance(value, str) and value.startswith(_PREFIX):
            name = value[len(_PREFIX):]
            self._asset_path(name)
            if self.origin is None:
                raise ValueError("public_origin is required for local CMS assets")
            return self.origin + "/content/api/assets/" + quote(name)
        return value

    def page(self, now, locale=None):
        if locale is not None and not _LOCALE.fullmatch(locale):
            raise ApiError(400, Err.INVALID_PAYLOAD, "Invalid content locale.")
        if self.snapshot is not None:
            result = self._resolve(self.snapshot)
            if locale is not None:
                result["_locale"] = locale
            return result
        # These are local service notices; they do not claim to reproduce a rotation.
        message = {"title": "Welcome to the local server",
                   "body": "You are connected to the local Fortnite server.",
                   "image": "", "hidden": False}
        return {"_title": "Fortnite Game", "_activeDate": now, "lastModified": now,
                "_locale": locale or "en-US",
                "battleroyalenews": {"_title": "Battle Royale News",
                                    "news": {"_type": "Battle Royale News",
                                             "messages": [copy.deepcopy(message)]},
                                    "motds": {"_type": "Motd", "motds": []}}}

    def asset(self, name):
        try:
            path = self._asset_path(name)
            data = path.read_bytes()
        except (OSError, ValueError):
            raise ApiError(404, Err.NOT_FOUND, "Content asset not found.") from None
        content_type = "image/png" if path.suffix.lower() == ".png" else "image/jpeg"
        return Raw(data, content_type)
