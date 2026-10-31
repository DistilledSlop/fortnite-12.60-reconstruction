"""Cloud storage: system files from a local directory, and per-account user files in memory.

Evidence:
  observed  GET /fortnite/api/cloudstorage/system and .../system/{filename}, and
            GET/PUT .../cloudstorage/user/{id}[/{filename}] paths in the route list;
            the field names uniqueFilename, filename, hash256, length, contentType;
            the error codes file_not_found, user_storage_data_not_found.
  inferred  the hash (sha1), storageType, storageIds, doNotCache and uploaded fields,
            the user-file list shape, and the 204 answer to PUT.

System files come from --cloudstorage-dir. The default is no directory, so the
listing is []. That keeps the client from downloading and applying config it has not
been shown to need. Do not point it at the extracted DefaultEngine.ini unless you mean to.
"""

import hashlib
import mimetypes
import re
from pathlib import Path

from api_common import ApiError, Err, Raw

FILENAME = re.compile(r"[A-Za-z0-9][A-Za-z0-9._-]{0,127}")


class CloudStorageService:
    def __init__(self, ctx, oauth, system_dir=None):
        self.ctx = ctx
        self.oauth = oauth
        self.system_dir = Path(system_dir) if system_dir else None
        self._user = {}

    def _system_files(self):
        """Regular files in the system directory, sorted by name. Hidden files are skipped."""
        if self.system_dir is None or not self.system_dir.is_dir():
            return {}
        files = {}
        for path in sorted(self.system_dir.iterdir()):
            if not path.is_file() or not FILENAME.fullmatch(path.name):
                continue
            files[path.name] = path
        return files

    @staticmethod
    def _describe(name, data, uploaded, ctx):
        return {
            "uniqueFilename": name,
            "filename": name,
            "hash": hashlib.sha1(data).hexdigest(),
            "hash256": hashlib.sha256(data).hexdigest(),
            "length": len(data),
            "contentType": mimetypes.guess_type(name)[0] or "application/octet-stream",
            "uploaded": ctx.iso(uploaded),
            "storageType": "S3",
            "storageIds": {},
            "doNotCache": False,
        }

    # Handlers ----------------------------------------------------------

    def system_list(self, req, match):
        listing = []
        for name, path in self._system_files().items():
            stat = path.stat()
            listing.append(self._describe(name, path.read_bytes(), stat.st_mtime, self.ctx))
        return 200, listing

    def system_file(self, req, match):
        name = match.group(1)
        path = self._system_files().get(name)
        if path is None:
            raise ApiError(404, Err.CLOUD_FILE_NOT_FOUND, "The system file was not found.")
        return 200, Raw(path.read_bytes(), mimetypes.guess_type(name)[0] or "application/octet-stream")

    def user_list(self, req, match):
        account_id = match.group(1)
        self.oauth.authenticate(req, account_id=account_id)
        stored = self._user.get(account_id, {})
        return 200, [self._describe(name, data, uploaded, self.ctx)
                     for name, (data, uploaded) in sorted(stored.items())]

    def user_get(self, req, match):
        account_id, name = match.group(1), match.group(2)
        self.oauth.authenticate(req, account_id=account_id)
        entry = self._user.get(account_id, {}).get(name)
        if entry is None:
            raise ApiError(404, Err.CLOUD_FILE_NOT_FOUND, "The user storage file was not found.")
        data, _ = entry
        return 200, Raw(data, "application/octet-stream")

    def user_put(self, req, match):
        account_id, name = match.group(1), match.group(2)
        self.oauth.authenticate(req, account_id=account_id)
        if not FILENAME.fullmatch(name):
            raise ApiError(400, Err.INVALID_PAYLOAD, "The file name is not valid.")
        self._user.setdefault(account_id, {})[name] = (bytes(req.body), self.ctx.now())
        return 204, None

    def register(self, router):
        router.add("GET", r"/fortnite/api/cloudstorage/system", self.system_list,
                   name="cloudstorage.system.list", status="observed")
        router.add("GET", r"/fortnite/api/cloudstorage/system/([^/]+)", self.system_file,
                   name="cloudstorage.system.file", status="observed")
        router.add("GET", r"/fortnite/api/cloudstorage/user/([^/]+)", self.user_list,
                   name="cloudstorage.user.list", status="observed")
        router.add("GET", r"/fortnite/api/cloudstorage/user/([^/]+)/([^/]+)", self.user_get,
                   name="cloudstorage.user.file", status="observed")
        router.add("PUT", r"/fortnite/api/cloudstorage/user/([^/]+)/([^/]+)", self.user_put,
                   name="cloudstorage.user.put", status="observed")
