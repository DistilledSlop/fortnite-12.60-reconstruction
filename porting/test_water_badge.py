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


class CcusageTest(unittest.TestCase):
    PAYLOAD = {'daily': [{'period': '2026-11-01', 'agent': 'all', 'agents': [
        {'agent': 'claude', 'inputTokens': 10, 'cacheReadTokens': 100, 'cacheCreationTokens': 5,
         'outputTokens': 20, 'totalTokens': 135,
         'modelBreakdowns': [
             {'modelName': 'claude-haiku-5-5', 'inputTokens': 4, 'cacheReadTokens': 60, 'cacheCreationTokens': 2, 'outputTokens': 10},
             {'modelName': 'claude-sonnet-5-5', 'inputTokens': 6, 'cacheReadTokens': 40, 'cacheCreationTokens': 3, 'outputTokens': 10}]},
        {'agent': 'antigravity', 'inputTokens': 7, 'cacheReadTokens': 0, 'cacheCreationTokens': 0,
         'outputTokens': 3, 'totalTokens': 12,
         'modelBreakdowns': [{'modelName': 'gemini-3.8-flash-low', 'inputTokens': 7, 'cacheReadTokens': 0,
                              'cacheCreationTokens': 0, 'outputTokens': 3}]}]}]}
    WINDOW = ('2026-10-30', '2026-11-06')

    def test_normalization_maps_providers_and_reconciles_total_tokens(self):
        rows = {(r['client'], r['model']): r for r in water.ccusage_records(self.PAYLOAD, *self.WINDOW)}
        self.assertEqual(len(rows), 3)
        haiku = rows[('claude', 'claude-haiku-5-5')]
        self.assertEqual((haiku['provider'], haiku['input_tokens'], haiku['cache_read_tokens'],
                          haiku['cache_write_tokens'], haiku['output_tokens'], haiku['reasoning_tokens']), ('anthropic', 4, 60, 2, 10, 0))
        self.assertEqual(rows[('antigravity', 'gemini-3.8-flash-low')]['provider'], 'google')
        # antigravity totalTokens (12) exceeds its reported fields (10) by 2: recorded as reasoning.
        self.assertEqual(rows[('antigravity', 'gemini-3.8-flash-low')]['reasoning_tokens'], 2)
        records = list(rows.values())
        self.assertEqual(sum(sum(r[k] for k in water.COUNTS) for r in records), 135 + 12)
        self.assertTrue(all(r['requests'] == 0 and r['method'] == 'ccusage' for r in records))
        self.assertTrue(all(r['timestamp'] == '2026-11-01T12:00:00+00:00' for r in records))
        for r in records:
            water.clean_record(r)

    def test_unknown_provider_and_out_of_window_days(self):
        self.assertEqual(water.ccusage_provider('gpt-6-sol'), 'openai')
        self.assertEqual(water.ccusage_provider('qwen3.8-27b'), 'unknown')
        self.assertEqual(water.ccusage_records(self.PAYLOAD, '2026-11-02', '2026-11-06'), [])

    def test_rejects_agent_totals_that_disagree_with_breakdowns(self):
        bad = json.loads(json.dumps(self.PAYLOAD))
        bad['daily'][0]['agents'][0]['outputTokens'] = 21
        with self.assertRaises(ValueError):
            water.ccusage_records(bad, *self.WINDOW)
        with self.assertRaises(ValueError):
            water.ccusage_records({'daily': [{'period': '2026-11-01', 'agent': 'all'}]}, *self.WINDOW)

    def test_residual_split_is_exact(self):
        parts = water._split_residual(5, [1, 3])
        self.assertEqual(sum(parts), 5)
        self.assertEqual(water._split_residual(4, [0, 0]), [4, 0])

    def test_ccusage_history_replaces_window_only(self):
        base = dict(provider='openai', model='gpt-6-sol', input_tokens=1, cache_read_tokens=0, cache_write_tokens=0,
                    output_tokens=0, reasoning_tokens=0, requests=0)
        rows = [dict(base, source='c' * 24, timestamp='2026-11-01T12:00:00+00:00', method='ccusage'),
                dict(base, source='d' * 24, timestamp='2026-11-01T08:00:00+00:00'),
                dict(base, source='e' * 24, timestamp='2026-10-29T08:00:00+00:00')]
        kept = water.scoped_history(rows, self.WINDOW)
        self.assertEqual({r['source'] for r in kept}, {'e' * 24})
        default = water.scoped_history(rows, None)
        self.assertEqual({r['source'] for r in default}, {'d' * 24, 'e' * 24})
        with self.assertRaises(ValueError):
            water.clean_record(dict(base, source='f' * 24, method='other'))

    def test_report_states_requests_unavailable(self):
        profiles = json.loads((water.HERE / 'water_profiles.json').read_text())
        record = dict(provider='anthropic', model='claude-haiku-5-5', client='claude', source='a' * 24,
                      input_tokens=1000, cache_read_tokens=0, cache_write_tokens=0, output_tokens=500,
                      reasoning_tokens=0, requests=0, method='ccusage', timestamp='2026-11-01T12:00:00+00:00')
        coverage = {'ccusage': {'window': list(self.WINDOW), 'request_counts': 'not reported by ccusage'}}
        data = water.summarize([record], coverage, profiles, '2026-11-06T00:00:00+00:00')
        self.assertFalse(data['requests_reported'])
        self.assertIsNone(data['recorded_requests'])
        self.assertTrue(data['scope'].startswith('Cloud agent usage detected by ccusage'))
        text = water.report(data)
        self.assertIn('Request counts are not reported by ccusage', text)
        self.assertIn('| Provider / model | Client |', text)
        self.assertIn('project scoping is unavailable', ' '.join(data['limitations']))


def _opencode_payload(total, qwen_out=50, deepseek_out=5):
    """One OpenCode agent-day: local qwen (excluded in tests) plus cloud deepseek-v4.1-flash-free."""
    breakdowns = [
        {'modelName': 'qwen3.8-27b', 'inputTokens': 10, 'cacheReadTokens': 100, 'cacheCreationTokens': 0, 'outputTokens': qwen_out},
        {'modelName': 'deepseek-v4.1-flash-free', 'inputTokens': 5, 'cacheReadTokens': 0, 'cacheCreationTokens': 0, 'outputTokens': deepseek_out}]
    agent = {'agent': 'opencode', 'inputTokens': 15, 'cacheReadTokens': 100, 'cacheCreationTokens': 0,
             'outputTokens': qwen_out + deepseek_out, 'modelBreakdowns': breakdowns}
    if total is not None:
        agent['totalTokens'] = total
    return {'daily': [{'period': '2026-11-02', 'agent': 'all', 'agents': [agent]}]}


class LocalExclusionTest(unittest.TestCase):
    WINDOW = ('2026-10-30', '2026-11-06')
    LOCAL = frozenset({'qwen3.8-27b'})

    def test_excluded_model_is_dropped_and_residual_does_not_leak(self):
        # 160 excluded (qwen) + 10 kept; totalTokens 170 means no residual at all.
        stats = {}
        recs = water.ccusage_records(_opencode_payload(170), *self.WINDOW, excluded=self.LOCAL, stats=stats)
        self.assertEqual([r['model'] for r in recs], ['deepseek-v4.1-flash-free'])
        self.assertEqual(recs[0]['reasoning_tokens'], 0)
        self.assertEqual(stats, {'excluded': {'qwen3.8-27b': 160}, 'unattributed': 0})

    def test_residual_goes_to_kept_model_only_and_totals_reconcile(self):
        stats = {}
        payload = _opencode_payload(173)  # 3 unexplained tokens
        recs = water.ccusage_records(payload, *self.WINDOW, excluded=self.LOCAL, stats=stats)
        self.assertEqual(len(recs), 1)
        self.assertEqual(recs[0]['reasoning_tokens'], 3)
        kept = sum(sum(r[k] for k in water.COUNTS) for r in recs)
        self.assertEqual(kept + stats['excluded']['qwen3.8-27b'] + stats['unattributed'], payload['daily'][0]['agents'][0]['totalTokens'])

    def test_residual_with_only_excluded_models_is_unattributed(self):
        payload = _opencode_payload(None)
        payload['daily'][0]['agents'][0]['modelBreakdowns'] = payload['daily'][0]['agents'][0]['modelBreakdowns'][:1]
        payload['daily'][0]['agents'][0].update(inputTokens=10, cacheReadTokens=100, outputTokens=50, totalTokens=164)
        stats = {}
        recs = water.ccusage_records(payload, *self.WINDOW, excluded=self.LOCAL, stats=stats)
        self.assertEqual(recs, [])
        self.assertEqual(stats['unattributed'], 4)

    def test_local_models_come_from_profile_list_when_db_is_missing(self):
        profiles = json.loads((water.HERE / 'water_profiles.json').read_text())
        names, status = water.local_models(profiles, '/nonexistent/opencode.db')
        self.assertEqual(status, 'missing')
        self.assertTrue({'qwen3.8-27b', 'deepseek-v4.1-flash-1017-coder-iq2xxs-remote', 'ds41'} <= names)
        self.assertNotIn('deepseek-v4.1-flash-free', names)

    def test_local_models_read_llamacpp_and_local_provider_ids(self):
        import sqlite3
        import tempfile
        with tempfile.TemporaryDirectory() as tmp:
            db = Path(tmp) / 'opencode.db'
            with sqlite3.connect(db) as conn:
                conn.execute('CREATE TABLE session_message (id TEXT, data TEXT)')
                rows = [{'model': {'providerID': 'llamacpp', 'id': 'new-local-7b'}, 'tokens': {}},
                        {'model': {'providerID': 'opencode', 'id': 'deepseek-v4.1-flash-free'}, 'tokens': {}},
                        {'model': {'providerID': 'local', 'id': 'other-local'}},
                        'not json']
                for i, row in enumerate(rows):
                    conn.execute('INSERT INTO session_message VALUES (?, ?)', (str(i), row if isinstance(row, str) else json.dumps(row)))
            names, status = water.local_models({'local_models': ['qwen3.8-27b']}, db)
        self.assertEqual(status, 'read')
        self.assertEqual(names, {'qwen3.8-27b', 'new-local-7b', 'other-local'})

    def test_report_lists_excluded_local_models(self):
        profiles = json.loads((water.HERE / 'water_profiles.json').read_text())
        record = dict(provider='anthropic', model='claude-haiku-5-5', client='claude', source='a' * 24,
                      input_tokens=1000, cache_read_tokens=0, cache_write_tokens=0, output_tokens=500,
                      reasoning_tokens=0, requests=0, method='ccusage', timestamp='2026-11-01T12:00:00+00:00')
        coverage = {'ccusage': {'local_excluded_tokens': {'qwen3.8-27b': 65000000}, 'local_models': ['qwen3.8-27b']}}
        data = water.summarize([record], coverage, profiles, '2026-11-06T00:00:00+00:00')
        self.assertIn('qwen3.8-27b 65.0M tokens', water.report(data))
        self.assertIn('Local models excluded by name: qwen3.8-27b', ' '.join(data['limitations']))


class SourceSelectionTest(unittest.TestCase):
    def setUp(self):
        self.original_first, self.original_run = water.first_commit_date, water.run_ccusage
        water.first_commit_date = lambda repo: '2026-10-30'
        self.calls = []

        def fake_run(binary, since, until):
            self.calls.append((since, until))
            return CcusageTest.PAYLOAD
        water.run_ccusage = fake_run

    def tearDown(self):
        water.first_commit_date, water.run_ccusage = self.original_first, self.original_run

    def test_default_is_ccusage_from_first_commit_to_today(self):
        args = water.build_parser().parse_args(['--opencode-db', '/nonexistent/opencode.db'])
        window = water.source_plan(args)
        self.assertEqual(window[0], '2026-10-30')
        self.assertRegex(window[1], r'^\d{4}-\d{2}-\d{2}$')
        records, coverage = water.gather_usage(args, window)
        self.assertEqual(self.calls, [window])
        self.assertIn('ccusage', coverage)
        self.assertEqual({r['method'] for r in records}, {'ccusage'})

    def test_ccusage_alias_is_a_noop(self):
        args = water.build_parser().parse_args(['--ccusage'])
        self.assertEqual(water.source_plan(args), water.source_plan(water.build_parser().parse_args([])))

    def test_logs_flag_selects_collectors_and_rejects_ccusage_options(self):
        import water_collectors
        original = water_collectors.collect_usage
        water_collectors.collect_usage = lambda **kw: ([{'marker': 'logs'}], {'codex': {}})
        try:
            args = water.build_parser().parse_args(['--logs'])
            self.assertIsNone(water.source_plan(args))
            records, coverage = water.gather_usage(args, None)
            self.assertEqual(records, [{'marker': 'logs'}])
            self.assertEqual(self.calls, [])
        finally:
            water_collectors.collect_usage = original
        for argv in (['--logs', '--since', '2026-11-01'], ['--logs', '--until', '2026-11-01'],
                     ['--logs', '--ccusage-bin', '/x/ccusage'], ['--logs', '--ccusage']):
            with self.assertRaises(ValueError):
                water.source_plan(water.build_parser().parse_args(argv))

    def test_invalid_window_is_rejected(self):
        with self.assertRaises(ValueError):
            water.source_plan(water.build_parser().parse_args(['--since', '2026-11-05', '--until', '2026-11-01']))
        with self.assertRaises(ValueError):
            water.source_plan(water.build_parser().parse_args(['--since', '11/01/2026']))


if __name__ == '__main__':
    unittest.main()
