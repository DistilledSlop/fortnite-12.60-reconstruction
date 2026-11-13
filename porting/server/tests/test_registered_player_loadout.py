#!/usr/bin/env python3
"""Focused contract checks for authenticated /local/gameserver/<sid>/player/<account> payloads."""

from __future__ import annotations

import json
import pathlib
import re
import sys
import tempfile
import unittest

if __package__:
    from . import _bootstrap
else:
    sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
    import _bootstrap

from api_common import ApiError, Request  # noqa: E402
from backend import Backend  # noqa: E402


class RegisteredPlayerLoadoutContractTests(unittest.TestCase):
    def setUp(self) -> None:
        self._tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self._tmp.cleanup)
        self.secret = "test-only-gameserver-secret"
        self.backend = Backend(state_dir=self._tmp.name, gameserver_secret=self.secret)
        self.acct_a = self.backend.accounts.create("AlphaDisplay")["id"]
        self.acct_b = self.backend.accounts.create("BetaDisplay")["id"]

        reg = self.backend.gameservers.register_server(Request(
            "POST",
            "/local/gameserver/register",
            body=json.dumps({
                "address": "127.0.0.1",
                "port": 7777,
                "playlists": ["Playlist_DefaultSolo"],
                "maxPlayers": 2,
                "buildUniqueId": "12896485",
            }).encode(),
            headers={"Authorization": "Bearer " + self.secret},
        ))[1]
        self.sid = reg["sessionId"]
        self.backend.gameservers.update_state(
            Request(
                "POST",
                f"/local/gameserver/{self.sid}/state",
                body=json.dumps({"phase": "warmup", "numPlayers": 0, "joinable": True}).encode(),
                headers={"Authorization": "Bearer " + self.secret},
            ),
            self.sid,
        )
        self.backend.gameservers.reserve("Playlist_DefaultSolo", [self.acct_a, self.acct_b], "12896485")

    def _fetch_player(self, account_id: str, *, secret: str | None = None) -> tuple[int, dict]:
        tok = self.secret if secret is None else secret
        req = Request(
            "GET",
            f"/local/gameserver/{self.sid}/player/{account_id}",
            headers={"Authorization": "Bearer " + tok},
        )
        m = re.fullmatch(r"/local/gameserver/([^/]+)/player/([^/]+)", req.path)
        self.assertIsNotNone(m)
        return self.backend.gameplay.player(req, m)

    def test_distinct_accounts_same_display_name_keep_selected_characters_isolated(self) -> None:
        profile_b = self.backend.profiles.profile(self.acct_b, "athena")
        locker_b = profile_b["stats"]["attributes"]["last_applied_loadout"]
        granted = self.backend.profiles.grant_item(
            self.acct_b,
            "athena",
            "AthenaCharacter:CID_690_Athena_Commando_F_Photographer",
            {"variants": [{"channel": "Progressive", "active": "Stage1", "owned": ["Stage1", "Stage2"]}]},
        )
        self.backend.profiles.run(
            self.acct_b,
            "athena",
            "SetCosmeticLockerSlot",
            {
                "lockerItem": locker_b,
                "category": "Character",
                "itemToSlot": granted,
                "slotIndex": 0,
                "variantUpdates": [{"channel": "Progressive", "active": "Stage2", "owned": []}],
            },
        )

        code_a, body_a = self._fetch_player(self.acct_a)
        code_b, body_b = self._fetch_player(self.acct_b)
        self.assertEqual(code_a, 200)
        self.assertEqual(code_b, 200)
        self.assertEqual(body_a["accountId"], self.acct_a)
        self.assertEqual(body_b["accountId"], self.acct_b)
        self.assertNotEqual(body_a["accountId"], body_b["accountId"])
        self.assertEqual(body_a["cosmetics"]["Character"], ["AthenaCharacter:cid_001_athena_commando_f_default"])
        self.assertEqual(body_a["variants"]["Character"], [])
        self.assertEqual(
            body_b["cosmetics"]["Character"],
            ["AthenaCharacter:CID_690_Athena_Commando_F_Photographer"],
        )
        self.assertEqual(
            body_b["variants"]["Character"],
            [{"variants": [{"channel": "Progressive", "active": "Stage2", "owned": ["Stage1", "Stage2"]}]}],
        )

    def test_rejects_bad_auth_and_unreserved_account(self) -> None:
        with self.assertRaises(ApiError) as auth_err:
            self._fetch_player(self.acct_a, secret="wrong")
        self.assertEqual(auth_err.exception.status, 401)

        acct_c = self.backend.accounts.create("GammaDisplay")["id"]
        with self.assertRaises(ApiError) as perm_err:
            self._fetch_player(acct_c)
        self.assertEqual(perm_err.exception.status, 403)


if __name__ == "__main__":
    unittest.main()
