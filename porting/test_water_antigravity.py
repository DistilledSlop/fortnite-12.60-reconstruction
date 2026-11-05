import hashlib
import json
from pathlib import Path
import sqlite3
import tempfile
import unittest

from water_antigravity import _wire, collect_antigravity


def varint(n):
    result = bytearray()
    while n > 127:
        result.append((n & 127) | 128)
        n >>= 7
    result.append(n)
    return bytes(result)


def number(field, value):
    return varint(field << 3) + varint(value)


def data(field, value):
    if isinstance(value, str):
        value = value.encode()
    return varint((field << 3) | 2) + varint(len(value)) + value


def usage(provider, identity, inp=100, out=30, read=40, think=10, assigned=''):
    return (number(2, inp) + number(3, out) + number(5, read) + number(6, provider)
            + number(9, think) + data(11, identity) + (data(12, assigned) if assigned else b''))


class TestAntigravity(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.root = Path(self.tmp.name)
        (self.root / 'conversations').mkdir()
        with sqlite3.connect(self.root / 'conversation_summaries.db') as conn:
            conn.execute('CREATE TABLE conversation_summaries (conversation_id TEXT, workspace_uris TEXT)')

    def tearDown(self):
        self.tmp.cleanup()

    def add(self, cid, uris, steps=(), gens=()):
        with sqlite3.connect(self.root / 'conversation_summaries.db') as conn:
            conn.execute('INSERT INTO conversation_summaries VALUES (?,?)', (cid, json.dumps(uris)))
        with sqlite3.connect(self.root / 'conversations' / (cid + '.db')) as conn:
            conn.execute('CREATE TABLE steps (idx INTEGER, metadata BLOB)')
            conn.execute('CREATE TABLE gen_metadata (idx INTEGER, data BLOB)')
            conn.executemany('INSERT INTO steps VALUES (?,?)', enumerate(steps))
            conn.executemany('INSERT INTO gen_metadata VALUES (?,?)', enumerate(gens))

    def collect(self):
        return collect_antigravity(self.root, ['/opt/fortnite'], '/opt/fortnite')

    def test_duplicate_metadata_scope_and_subsets(self):
        u = usage(24, 'resp-1')
        self.add('project', ['file:///opt/fortnite/main'],
                 [data(1, number(1, 1700000000)) + data(9, u) + data(4, 'PRIVATE PROMPT')],
                 [data(1, data(4, u) + data(22, 'gemini-flash') + data(1, 'SECRET'))])
        self.add('research', ['file:///opt/fortnite/Fortnite14Research'], [data(9, u)])
        self.add('other', ['file:///other'], [data(9, u)])
        records, cov = self.collect()
        self.assertEqual(cov['usage_records'], 1)
        self.assertEqual(cov['scoped_files'], 1)
        rec = records[0]
        self.assertEqual((rec['provider'], rec['model']), ('google', 'gemini-flash'))
        self.assertEqual([rec[k] for k in ('input_tokens', 'cache_read_tokens', 'output_tokens', 'reasoning_tokens')], [100, 40, 20, 10])
        self.assertEqual(rec['timestamp'], '2023-11-14T22:13:20Z')
        self.assertNotIn('SECRET', repr(records))
        self.assertNotIn('PRIVATE', repr(records))
        self.assertNotIn('resp-1', repr(records))

    def test_anthropic_provider_name_enriches_and_matches_native_hash(self):
        u = usage(3, 'internal-response', assigned='msg_test')
        self.add('one', ['file:///opt/fortnite'], [data(9, u)],
                 [data(1, data(4, u) + data(22, 'claude-sonnet'))])
        rec = self.collect()[0][0]
        self.assertEqual(rec['provider'], 'anthropic')
        self.assertEqual(rec['input_tokens'], 100)
        self.assertEqual(rec['source'], hashlib.sha256(b'claude:msg_test').hexdigest()[:24])

    def test_bad_wire_and_missing_database_reported(self):
        self.add('broken', ['file:///opt/fortnite'], [b'\x4a\xff'], [])
        with sqlite3.connect(self.root / 'conversation_summaries.db') as conn:
            conn.execute('INSERT INTO conversation_summaries VALUES (?,?)', ('missing', '["file:///opt/fortnite"]'))
        records, cov = self.collect()
        self.assertFalse(records)
        self.assertEqual(cov['malformed_metadata'], 1)
        self.assertEqual(cov['missing_databases'], 1)
        self.assertEqual(cov['missing_usage_files'], 1)

    def test_wire_rejects_truncation(self):
        with self.assertRaises(ValueError):
            _wire(b'\x0a\x03x')


if __name__ == '__main__':
    unittest.main()
