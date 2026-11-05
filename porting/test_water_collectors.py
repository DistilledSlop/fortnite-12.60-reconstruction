import json
import sqlite3
import tempfile
import unittest
from pathlib import Path

import water_collectors as wc


class CollectorsTest(unittest.TestCase):
    def test_codex_response_ids_forks_models_and_cumulative_fallback(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            def write(name, data):
                (root / name).write_text(''.join(json.dumps(x) + '\n' for x in data))
            meta = {'type': 'session_meta', 'payload': {'cwd': '/project/repo'}}
            ctx = {'type': 'turn_context', 'payload': {'model': 'm1'}}
            usage = {'input_tokens': 100, 'cached_input_tokens': 30, 'cache_write_input_tokens': 10,
                     'output_tokens': 20, 'reasoning_output_tokens': 5}
            item = {'type': 'token_usage_record', 'payload': {'response_id': 'r1', 'usage': usage}}
            write('a.jsonl', [meta, ctx, item])
            write('b.jsonl', [meta, ctx, item])
            write('c.jsonl', [meta, ctx, {'type': 'event_msg', 'timestamp': 't1', 'payload': {'type': 'token_count', 'info': {'total_token_usage': usage}}}])
            write('outside.jsonl', [{'type': 'session_meta', 'payload': {'cwd': '/other'}}, item])
            records, cov = wc._codex(root, ['/project'], '/project')
            self.assertEqual(len(records), 2)
            self.assertEqual((records[0]['input_tokens'], records[0]['cache_read_tokens'], records[0]['cache_write_tokens']), (60, 30, 10))
            self.assertEqual((records[0]['output_tokens'], records[0]['reasoning_tokens']), (15, 5))
            self.assertEqual(sum(r['requests'] for r in records), 1)
            self.assertEqual(cov['fallback_files'], 1)

    def test_claude_stream_snapshots_and_project_scope(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            def entry(count, cwd='/project/repo'):
                return {'type': 'assistant', 'cwd': cwd, 'message': {'id': 'one', 'model': 'sonnet', 'usage': {
                    'input_tokens': 2, 'cache_read_input_tokens': 20, 'cache_creation_input_tokens': 4,
                    'output_tokens': count, 'output_tokens_details': {'thinking_tokens': 3}}}}
            (root / 'one.jsonl').write_text('\n'.join(json.dumps(entry(n)) for n in (10, 20)))
            (root / 'two.jsonl').write_text(json.dumps(entry(20)) + '\n' + json.dumps({'type': 'assistant', 'cwd': '/elsewhere', 'message': {'id': 'outside', 'usage': {'input_tokens': 999}}}))
            records, cov = wc._claude(root, ['/project'], '/project')
            self.assertEqual(len(records), 1)
            self.assertEqual((records[0]['output_tokens'], records[0]['reasoning_tokens']), (17, 3))
            self.assertEqual(records[0]['input_tokens'], 2)
            self.assertEqual(cov['scoped_files'], 2)

    def test_codex_scope_duplicate_and_fallback_overlap(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            def line(obj): return json.dumps(obj) + '\n'
            meta = {'type': 'session_meta', 'payload': {'cwd': '/project'}}
            total = {'input_tokens': 100, 'cached_input_tokens': 20, 'output_tokens': 5}
            usage = {'input_tokens': 100, 'cached_input_tokens': 20, 'output_tokens': 5}
            direct = lambda u: {'type': 'token_usage_record', 'payload': {'response_id': 'a', 'thread_token_usage': total, 'usage': u}}
            (root / 'a.jsonl').write_text(''.join(map(line, [meta, direct(dict(usage, output_tokens=2)), direct(usage), {'type': 'turn_context', 'payload': {'cwd': '/elsewhere'}}, {'type': 'token_usage_record', 'payload': {'response_id': 'outside', 'usage': usage}}])))
            (root / 'b.jsonl').write_text(''.join(map(line, [meta, {'type': 'event_msg', 'timestamp': 'x', 'payload': {'type': 'token_count', 'info': {'total_token_usage': total}}}])))
            records, cov = wc._codex(root, ['/project'], '/project')
            self.assertEqual(len(records), 1)
            self.assertEqual(records[0]['output_tokens'], 5)
            self.assertEqual(records[0]['provider'], 'openai')

    def test_mixed_direct_and_cumulative_missing_id_alias(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            first = {'input_tokens': 100, 'output_tokens': 10}
            second = {'input_tokens': 120, 'output_tokens': 12}
            def event(ts, total):
                return {'timestamp': ts, 'type': 'event_msg', 'payload': {'type': 'token_count', 'info': {'total_token_usage': total}}}
            rows = [{'type': 'session_meta', 'payload': {'cwd': '/project'}},
                    {'type': 'token_usage_record', 'payload': {'response_id': 'one', 'thread_token_usage': first, 'usage': first}}, event('t1', first),
                    {'type': 'token_usage_record', 'payload': {'thread_token_usage': second, 'usage': {'input_tokens': 20, 'output_tokens': 2}}}, event('t2', second)]
            (root / 'x.jsonl').write_text(''.join(json.dumps(row) + '\n' for row in rows))
            records, cov = wc._codex(root, ['/project/repo'], '/project')
            direct = next(r for r in records if r['requests'])
            fallback = next(r for r in records if not r['requests'])
            self.assertEqual((fallback['input_tokens'], fallback['output_tokens']), (20, 2))
            self.assertEqual(direct['aliases'], [wc._hash('codex-event:t1:' + json.dumps(first, sort_keys=True))])
            self.assertEqual(cov['missing_response_ids'], 1)
            self.assertEqual(cov['unaccounted_events'], 0)

    def test_mixed_interval_only_adds_unobserved_residual(self):
        for direct_values, expected in [([(50, 50)], 50), ([(50, 50), (25, 100)], 25)]:
            with self.subTest(direct_values=direct_values), tempfile.TemporaryDirectory() as tmp:
                rows = [{'type': 'session_meta', 'payload': {'cwd': '/project'}}]
                for index, (amount, cumulative) in enumerate(direct_values):
                    rows.append({'type': 'token_usage_record', 'payload': {'response_id': str(index), 'usage': {'input_tokens': amount}, 'thread_token_usage': {'input_tokens': cumulative}}})
                rows.append({'type': 'event_msg', 'timestamp': 't', 'payload': {'type': 'token_count', 'info': {'total_token_usage': {'input_tokens': 100}}}})
                root = Path(tmp)
                (root / 'x.jsonl').write_text(''.join(json.dumps(r) + '\n' for r in rows))
                records, cov = wc._codex(root, ['/project/repo'], '/project')
                self.assertEqual(sum(r['input_tokens'] for r in records if not r['requests']), expected)
                self.assertEqual(cov['unaccounted_events'], 0)

    def test_streaming_reasoning_does_not_double_count_output(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            entries = [{'type': 'assistant', 'cwd': '/project', 'message': {'id': 'm', 'usage': {'output_tokens': 20, 'output_tokens_details': {'thinking_tokens': n}}}} for n in (0, 10)]
            (root / 'x.jsonl').write_text(''.join(json.dumps(r) + '\n' for r in entries))
            records, _ = wc._claude(root, ['/project/repo'], '/project')
            self.assertEqual((records[0]['output_tokens'], records[0]['reasoning_tokens']), (10, 10))

    def test_scope_parent_worktree_and_siblings(self):
        roots = ['/project/repo', '/elsewhere/worktree', '/project/gemini-package']
        self.assertTrue(wc._in_scope('/project', roots, '/project'))
        self.assertTrue(wc._in_scope('/project/repo/porting', roots, '/project'))
        self.assertTrue(wc._in_scope('/elsewhere/worktree', roots, '/project'))
        self.assertTrue(wc._in_scope('/project/gemini-package', roots, '/project'))
        self.assertFalse(wc._in_scope('/project/unrelated', roots, '/project'))
        self.assertFalse(wc._in_scope('/project/Fortnite14Research', roots, '/project'))

    def test_native_gemini_hash_and_token_subsets(self):
        with tempfile.TemporaryDirectory() as tmp:
            import hashlib
            root = Path(tmp)
            chats = root / 'tmp' / 'project' / 'chats'
            chats.mkdir(parents=True)
            digest = hashlib.sha256(b'/project').hexdigest()
            data = {'projectHash': digest, 'messages': [{'id': 'g', 'type': 'gemini', 'model': 'gemini-test', 'tokens': {'input': 100, 'cached': 40, 'output': 8, 'thoughts': 3}}]}
            (chats / 'session-one.json').write_text(json.dumps(data))
            records, cov = wc._gemini(root, ['/project'], '/project')
            self.assertEqual(len(records), 1)
            self.assertEqual((records[0]['input_tokens'], records[0]['output_tokens'], records[0]['reasoning_tokens']), (60, 8, 3))
            self.assertEqual(records[0]['provider'], 'google')

    def test_collect_usage_excludes_zero_records(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            codex = root / 'codex'
            codex.mkdir()
            records = [{'type': 'session_meta', 'payload': {'cwd': '/project'}},
                       {'type': 'token_usage_record', 'payload': {'response_id': 'zero', 'usage': {'input_tokens': 0, 'output_tokens': 0}}}]
            (codex / 'log.jsonl').write_text(''.join(json.dumps(r) + '\n' for r in records))
            result, cov = wc.collect_usage(codex_root=codex, claude_root=root / 'claude', opencode_db=root / 'none', gemini_root=root / 'gemini', antigravity_root=root / 'agy', project_root='/project', repo='/nonexistent')
            self.assertEqual(result, [])
            self.assertEqual(cov['codex']['excluded_zero_records'], 1)
            self.assertEqual(cov['codex']['usage_records'], 0)

    def test_opencode_scopes_and_reads_separate_counts(self):
        with tempfile.TemporaryDirectory() as tmp:
            db = Path(tmp) / 'db'
            with sqlite3.connect(db) as c:
                c.execute('create table session_v2 (id text, directory text)')
                c.execute('create table session_message (id text, session_id text, data text)')
                c.execute('insert into session_v2 values (?, ?)', ('s', '/project/repo'))
                data = {'model': {'id': 'qwen', 'providerID': 'local'}, 'tokens': {'input': 10, 'output': 4, 'reasoning': 2, 'cache': {'read': 3, 'write': 1}}}
                c.execute('insert into session_message values (?, ?, ?)', ('m', 's', json.dumps(data)))
            records, cov = wc._opencode(db, ['/project'], '/project')
            self.assertEqual(len(records), 1)
            self.assertEqual(records[0]['provider'], 'local')
            self.assertEqual(records[0]['reasoning_tokens'], 2)
            self.assertEqual(cov['scoped_files'], 1)


if __name__ == '__main__':
    unittest.main()
