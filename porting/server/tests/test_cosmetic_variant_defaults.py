"""Cooked default styles for gliders/outfits: grants, legacy repair on query, strict equip."""
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

GLIDER = 'AthenaGlider:Glider_ID_085_SkullTrooper'


class VariantDefaultTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.svc = Backend(state_dir=self.tmp.name).profiles
        self.account = 'variant-default-test'
        self.locker = self.svc.profile(self.account, 'athena')['stats']['attributes']['last_applied_loadout']

    def equip(self, item, active):
        return self.svc.run(self.account, 'athena', 'SetCosmeticLockerSlot', {
            'lockerItem': self.locker, 'category': 'Glider', 'itemToSlot': item, 'slotIndex': 0,
            'variantUpdates': [{'channel': 'Emissive', 'active': active, 'owned': [active]}]})

    def test_data_driven_grant(self):
        variants = cosmetic_attributes(GLIDER)['variants']
        self.assertEqual(variants[0]['channel'], 'Emissive')
        self.assertIn(variants[0]['active'], variants[0]['owned'])

    def test_legacy_item_repaired_on_query_and_equip_stays_strict(self):
        item = self.svc.grant_item(self.account, 'athena', GLIDER, {'variants': []})
        self.svc.query(self.account, 'athena')
        stored = self.svc.profile(self.account, 'athena')['items'][item]['attributes']['variants']
        active = stored[0]['active']
        self.equip(item, active)
        with self.assertRaises(ApiError):
            self.equip(item, 'NotAStyle')


if __name__ == '__main__':
    unittest.main()
