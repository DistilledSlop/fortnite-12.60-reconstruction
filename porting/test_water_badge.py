import importlib.util
import json
from pathlib import Path
import unittest

import water_badge as water


class EstimateTest(unittest.TestCase):
    def setUp(self):
        self.profiles = json.loads((water.HERE / 'water_profiles.json').read_text())
        self.record = dict(provider='openai', model='gpt-6.1-terra', source='a' * 24,
                           input_tokens=1000000, cache_read_tokens=2000000,
                           cache_write_tokens=1000000, output_tokens=1000000,
                           reasoning_tokens=1000000, requests=1)

    def test_disjoint_token_costs_and_water_denominators(self):
        r = water.estimate(self.record, self.profiles)
        # 1M input + 2M reads*0.1 + 1M writes*1.2, then 2M generated.
        energy = (1 + 2 * .1 + 1 * 1.2) * .25 + 2 * 1.25
        self.assertAlmostEqual(r['energy_kwh'][1], energy)
        self.assertAlmostEqual(r['cooling_liters'][1], energy / 1.2 * .36)
        self.assertAlmostEqual(r['electricity_liters'][1], energy * 4.52)
        self.assertLess(r['gallons'][0], r['gallons'][1])
        self.assertLess(r['gallons'][1], r['gallons'][2])

    def test_local_and_exact_model_override(self):
        self.record['provider'] = 'local'
        self.profiles['model_overrides'][self.record['model']] = {
            'input_kwh_per_million': [1, 2, 3], 'output_kwh_per_million': [3, 4, 5]}
        r = water.estimate(self.record, self.profiles)
        self.assertEqual(r['cooling_liters'], [0, 0, 0])
        self.assertAlmostEqual(r['energy_kwh'][1], 2.4 * 2 + 2 * 4)
        self.assertGreater(r['electricity_liters'][1], 0)

    def test_ledger_refresh_replaces_and_retains_history_without_private_fields(self):
        before = dict(self.record, output_tokens=5)
        current = dict(self.record, output_tokens=7, prompt='private text')
        other = dict(before, source='b' * 24)
        records = water.merge_records([before, other], [current, current])
        self.assertEqual(len(records), 2)
        self.assertEqual(records[0]['output_tokens'], 7)
        self.assertNotIn('prompt', json.dumps(records))
        self.assertEqual(records, water.merge_records(records, [current]))
        with self.assertRaises(ValueError):
            water.clean_record(dict(self.record, input_tokens=-1))
        with self.assertRaises(ValueError):
            water.clean_record(dict(self.record, source='/private/path'))

    def test_summary_and_unknown_profile(self):
        d = water.summarize([self.record], {'missing': True}, self.profiles, '2026-11-05T00:00:00Z')
        self.assertEqual(d['tokens'], 6000000)
        self.assertEqual(d['token_types']['reasoning_tokens'], 1000000)
        self.assertEqual(d['estimate'], {k: d['models'][0][k] for k in d['estimate']})
        self.assertEqual(water.model_tier('unrecognized'), 'unknown')
        self.assertIn('not a measurement', water.report(d))

    def test_direct_identity_supersedes_retained_fallback_and_reasoning_reclassification(self):
        fallback = dict(self.record, source='b' * 24, requests=0)
        direct = dict(self.record, aliases=['b' * 24])
        rows = water.merge_records([fallback], [direct])
        self.assertEqual(len(rows), 1)
        self.assertEqual(rows[0]['requests'], 1)
        unsplit = dict(self.record, output_tokens=2000000, reasoning_tokens=0)
        rows = water.merge_records([], [unsplit, self.record])
        self.assertEqual(rows[0]['output_tokens'] + rows[0]['reasoning_tokens'], 2000000)
        self.assertEqual(rows[0]['reasoning_tokens'], 1000000)

    def test_water_only_publication_preserves_other_edits(self):
        path = water.HERE / 'progress/publish.py'
        spec = importlib.util.spec_from_file_location('water_publish_test', path)
        pub = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(pub)
        pub.water_badge = lambda: ['<a href="report"><img alt="AI water est.: 1–9 gal" src="example"></a>']
        original = 'other edited content\n<img alt="water used: 20.6 gal" src="old">\nprogress edits'
        updated = pub.refresh_water_only(original)
        self.assertTrue(updated.startswith('other edited content\n'))
        self.assertTrue(updated.endswith('\nprogress edits'))
        self.assertEqual(pub.refresh_water_only(updated), updated)
        with self.assertRaises(ValueError):
            pub.refresh_water_only('missing badge')


if __name__ == '__main__':
    unittest.main()
