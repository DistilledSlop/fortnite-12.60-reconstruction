"""A granted item's caller-owned attributes cannot mutate persistent state."""

# Isolated mode omits the script directory from sys.path.
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import tempfile
import unittest

from backend import Backend


class ProfileGrantOwnershipTests(unittest.TestCase):
    def test_nested_attributes_are_owned_by_profile_and_survive_restart(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            attributes = {"variants": [{"channel": "Material", "owned": ["Blue"]}]}
            item_id = service.grant_item("account", "athena", "AthenaDance:test", attributes)
            before = service.query("account", "athena")["profileChanges"][0]["profile"]

            attributes["variants"][0]["owned"].append("Red")
            current = service.query("account", "athena")["profileChanges"][0]["profile"]
            restarted = Backend(state_dir=directory).profiles.query("account", "athena")
            restored = restarted["profileChanges"][0]["profile"]
            self.assertEqual(current, before)
            self.assertEqual(restored, before)
            self.assertEqual(restored["items"][item_id]["attributes"]["variants"][0]["owned"], ["Blue"])

    def test_empty_attributes_are_preserved(self):
        with tempfile.TemporaryDirectory() as directory:
            service = Backend(state_dir=directory).profiles
            item_id = service.grant_item("account", "athena", "AthenaDance:test", {})
            restored = Backend(state_dir=directory).profiles.profile("account", "athena")
            self.assertEqual(restored["items"][item_id]["attributes"], {})


if __name__ == "__main__":
    unittest.main()
