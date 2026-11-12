"""Exercise MCP revision recovery through the real HTTP handler."""

# Isolated mode omits the script directory from sys.path.
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import unittest
import sys
from pathlib import Path

if __package__:
    from . import test_services as support
else:
    import test_services as support


class ProfileRevisionRecoveryTests(support.ServiceTestCase):
    def test_stale_command_receives_resulting_snapshot_and_current_gets_delta(self):
        token = self.sign_in()["access_token"]
        path = f"/fortnite/api/game/v2/profile/{support.LOCAL}/client/"

        def command(name, rvn, body):
            status, result, _ = support.jcall(
                self.base, "POST", path + name + f"?profileId=athena&rvn={rvn}",
                token=token, body=body)
            self.assertEqual(status, 200, result)
            return result

        initial = command("QueryProfile", -1, {})["profileChanges"][0]["profile"]
        locker = initial["stats"]["attributes"]["last_applied_loadout"]
        current = command("SetCosmeticLockerName", initial["rvn"],
                          {"lockerItem": locker, "name": "First"})
        self.assertEqual(current["profileChanges"][0]["changeType"], "itemAttrChanged")
        self.assertEqual(current["profileChangesBaseRevision"], initial["rvn"])

        stale = command("SetCosmeticLockerName", initial["rvn"],
                        {"lockerItem": locker, "name": "Second"})
        self.assertEqual(len(stale["profileChanges"]), 1)
        change = stale["profileChanges"][0]
        self.assertEqual(change["changeType"], "fullProfileUpdate")
        self.assertEqual(change["profile"]["items"][locker]["attributes"]["locker_name"], "Second")
        self.assertEqual(change["profile"]["rvn"], stale["profileRevision"])
        self.assertEqual(stale["profileChangesBaseRevision"], stale["profileRevision"])

        # A stale no-op still needs a snapshot for the client to catch up.
        noop = command("SetCosmeticLockerName", -1, {"lockerItem": locker, "name": "Second"})
        self.assertEqual(noop["profileChanges"][0]["changeType"], "fullProfileUpdate")
        self.assertEqual(noop["profileRevision"], stale["profileRevision"])
        self.assertEqual(noop["profileCommandRevision"], stale["profileCommandRevision"] + 1)


if __name__ == "__main__":
    unittest.main()
