"""Release-day storefront reconstruction and purchase lookup invariants."""
import unittest

import catalog


class ReleaseDayShopTest(unittest.TestCase):
    def test_rotation_prices_and_grouping(self):
        shops = {s['name']: s['catalogEntries'] for s in catalog.storefronts()}
        expected = {
            'BRWeeklyStorefront': [('Lace', 1500), ('Paradox', 1500), ('Echo', 1200),
                ('Echo Jet', 800), ('Monks', 1200), ('Vision', 800), ('Echoes', 300),
                ('Equilibrium', 500), ('Inversion Blades', 500)],
            'BRDailyStorefront': [('Electro Shuffle', 800), ('Chromium', 1200),
                ('Fractal Zero', 500), ('Hitchhiker', 500), ('Jubilation', 200),
                ('Recon Ranger', 800)],
        }
        for name, rows in expected.items():
            self.assertEqual([(e['devName'], e['prices'][0]['finalPrice'])
                              for e in shops[name]], rows)
            for entry in shops[name]:
                self.assertTrue(entry['giftInfo']['bIsEnabled'])
                self.assertEqual(catalog.get_offer(entry['offerId']), entry)
        self.assertIsNone(catalog.get_offer('local-floss'))
        self.assertIsNone(catalog.get_offer('local-arctic-camo'))

    def test_outfit_bundles_and_isolation(self):
        entries = {e['devName']: e for s in catalog.storefronts() for e in s['catalogEntries']}
        for name, backpack in [('Lace', 'BID_190_StreetGothFemale'),
                               ('Paradox', 'BID_189_StreetGothMale'),
                               ('Echo', 'BID_489_TwinDark'),
                               ('Monks', 'BID_447_ToyMonkey')]:
            self.assertEqual(entries[name]['itemGrants'][1],
                             {'templateId': 'AthenaBackpack:' + backpack, 'quantity': 1})
        entries['Lace']['itemGrants'].clear()
        self.assertEqual(len(catalog.get_offer('local-20200520-lace')['itemGrants']), 2)
        self.assertIsNotNone(catalog.get_offer(catalog.LOCAL_CREDIT_OFFER))
        self.assertIsNotNone(catalog.get_offer(catalog.SEASON12_PASS_OFFER))
        self.assertIsNotNone(catalog.get_offer(catalog.SEASON12_LEVEL_OFFER))


if __name__ == '__main__':
    unittest.main()
