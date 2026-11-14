"""Cooked Agent Peely default ownership and legacy profile repair."""
if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap
import tempfile
import unittest
from api_common import ApiError
from backend import Backend
from catalog import cosmetic_attributes

PEELY = 'AthenaCharacter:CID_701_Athena_Commando_M_BananaAgent'
DEFAULT = {'channel': 'Progressive', 'active': 'Stage1', 'owned': ['Stage1']}

class PeelyDefaultTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.svc = Backend(state_dir=self.tmp.name).profiles
        self.account = 'peely-default-test'
        self.locker = self.profile()['stats']['attributes']['last_applied_loadout']

    def profile(self):
        return self.svc.profile(self.account, 'athena')

    def equip(self, item, active):
        return self.svc.run(self.account, 'athena', 'SetCosmeticLockerSlot', {
            'lockerItem': self.locker, 'category': 'Character', 'itemToSlot': item,
            'slotIndex': 0, 'variantUpdates': [{'channel': 'Progressive', 'active': active, 'owned': [active]}]})

    def test_new_grant_and_locked_or_wrong_cosmetic(self):
        self.assertEqual(cosmetic_attributes(PEELY)['variants'], [DEFAULT])
        peely = self.svc.grant_item(self.account, 'athena', PEELY, cosmetic_attributes(PEELY))
        self.equip(peely, 'Stage1')
        other = self.svc.grant_item(self.account, 'athena', 'AthenaCharacter:other', {'variants': []})
        for item, stage in ((peely, 'Stage2'), (other, 'Stage1')):
            before = self.profile()
            with self.assertRaises(ApiError): self.equip(item, stage)
            self.assertEqual(self.profile(), before)

    def test_login_repairs_legacy_and_preserves_owned_selection(self):
        peely = self.svc.grant_item(self.account, 'athena', PEELY, {'variants': []})
        self.svc.run(self.account, 'athena', 'ClientQuestLogin', {})
        self.assertEqual(self.profile()['items'][peely]['attributes']['variants'], [DEFAULT])
        self.equip(peely, 'Stage1')
        self.svc.run(self.account, 'athena', 'ClientQuestLogin', {})
        self.assertEqual(self.profile()['items'][peely]['attributes']['variants'], [DEFAULT])

    def test_existing_channel_keeps_selection_and_other_channels(self):
        existing = [{'channel': 'Progressive', 'active': 'Stage2', 'owned': ['Stage2']},
                    {'channel': 'Other', 'active': 'A', 'owned': ['A']}]
        peely = self.svc.grant_item(self.account, 'athena', PEELY, {'variants': existing})
        self.svc.run(self.account, 'athena', 'ClientQuestLogin', {})
        self.assertEqual(self.profile()['items'][peely]['attributes']['variants'], [
            {'channel': 'Progressive', 'active': 'Stage2', 'owned': ['Stage2', 'Stage1']}, existing[1]])
        self.equip(peely, 'Stage2')
