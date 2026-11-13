"""Competition download authentication and empty-calendar contract."""

if __package__:
    from . import _bootstrap
    from .test_services import ServiceTestCase, jcall
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap
    from test_services import ServiceTestCase, jcall

import json

from api_common import Err


class EventsDownloadTests(ServiceTestCase):
    def test_owner_download_and_route_boundaries(self):
        owner = self.sign_in()
        other = self.sign_in("local-username:EventsOther")
        path = "/api/v1/events/Fortnite/download/" + owner["account_id"]
        self.assertError(jcall(self.base, "GET", path), 401, Err.AUTH_REQUIRED)
        self.assertError(jcall(self.base, "GET", path, token=other["access_token"]),
                         403, Err.MISSING_PERMISSION)
        self.assertError(jcall(self.base, "POST", path, token=owner["access_token"]),
                         405, Err.METHOD_NOT_ALLOWED)
        self.assertError(jcall(self.base, "GET", path.replace("Fortnite", "Other"),
                               token=owner["access_token"]), 404, Err.NOT_FOUND)

        expected = {"player": {"gameId": "Fortnite", "accountId": owner["account_id"],
                               "tokens": [], "persistentScores": {}, "pendingPayouts": [],
                               "teams": {}}, "events": [], "templates": [], "scores": []}
        status, payload, _ = jcall(self.base, "GET", path +
                                   "?platform=Windows&region=NAE&teamAccountIds=a%2Cb",
                                   token=owner["access_token"])
        self.assertEqual((status, payload), (200, expected))
        self.assertNotIn(owner["access_token"], json.dumps(payload))
        self.assertNotIn(other["access_token"], json.dumps(payload))
        self.assertEqual(jcall(self.base, "GET", path, token=owner["access_token"])[:2],
                         (200, expected))

        headers = {"Authorization": "Bearer " + owner["access_token"]}
        _, first = self.backend.respond("GET", path, headers=headers)
        first["player"]["tokens"].append("modified")
        first["events"].append({})
        self.assertEqual(self.backend.respond("GET", path, headers=headers), (200, expected))
