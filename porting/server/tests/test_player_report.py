"""Player report route (toxicity service stand-in)."""
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap
import re
import tempfile
import types
import unittest
from api_common import ApiError
from backend import Backend


class PlayerReportTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.gp = Backend(state_dir=self.tmp.name).gameplay

    def call(self, session_account, reporter, reported):
        self.gp.oauth = types.SimpleNamespace(account_session=lambda req: types.SimpleNamespace(account_id=session_account))
        self.gp.accounts = types.SimpleNamespace(get=lambda a: {"id": a})
        match = re.match(r"(.+)/(.+)", f"{reporter}/{reported}")
        return self.gp.report_player(types.SimpleNamespace(json_object=lambda: {"reason": "x"}), match)

    def test_report_accepted_and_recorded(self):
        self.assertEqual(self.call("a", "a", "b"), (204, None))
        self.assertEqual(self.gp.reports[-1]["reported"], "b")

    def test_cannot_report_as_someone_else(self):
        with self.assertRaises(ApiError):
            self.call("a", "c", "b")


if __name__ == "__main__":
    unittest.main()
