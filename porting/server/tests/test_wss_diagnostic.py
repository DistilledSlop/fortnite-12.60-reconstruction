if __package__:
    from . import _bootstrap
else:
    import sys as _sys
    from pathlib import Path as _Path
    _sys.path.insert(0, str(_Path(__file__).resolve().parent))
    import _bootstrap

import unittest
from unittest import mock

import patch_wss_diagnostic as patch


class SparseMemory:
    def __init__(self):
        self.data = {}
        self.position = 0
        self.writes = []
        self.load(patch.BRANCH_CONTEXT_ADDRESS, patch.BRANCH_CONTEXT)
        self.load(patch.TARGET_CONTEXT_ADDRESS, patch.TARGET_CONTEXT)

    def load(self, address, value):
        self.data.update({address + i: byte for i, byte in enumerate(value)})

    def seek(self, address):
        self.position = address

    def read(self, length):
        result = bytes(self.data.get(self.position + i, 0) for i in range(length))
        self.position += length
        return result

    def write(self, value):
        self.writes.append((self.position, value))
        self.load(self.position, value)
        self.position += len(value)
        return len(value)


class WssDiagnosticTests(unittest.TestCase):
    def test_single_byte_patch_is_idempotent(self):
        mem = SparseMemory()
        self.assertEqual(patch.patch_memory(mem), 'applied')
        self.assertEqual(mem.writes, [(patch.BRANCH_ADDRESS, b'\xeb')])
        self.assertEqual(patch.patch_memory(mem), 'already')
        self.assertEqual(mem.writes, [(patch.BRANCH_ADDRESS, b'\xeb')])

    @mock.patch.object(patch, 'shipping_client_pids')
    @mock.patch.object(patch.os, 'readlink', return_value='net:[1]')
    @mock.patch.object(patch.os, 'geteuid', return_value=0)
    def test_host_namespace_is_rejected_before_process_lookup(self, _uid, _readlink, pids):
        with mock.patch.object(patch.sys, 'stderr'):
            self.assertEqual(patch.main(['1']), 1)
        pids.assert_not_called()

    def test_wrong_branch_or_target_is_never_written(self):
        for address in (patch.BRANCH_CONTEXT_ADDRESS, patch.TARGET_CONTEXT_ADDRESS):
            with self.subTest(address=address):
                mem = SparseMemory()
                mem.data[address] ^= 1
                before = mem.data.copy()
                self.assertEqual(patch.patch_memory(mem), 'mismatch')
                self.assertEqual(mem.data, before)
                self.assertEqual(mem.writes, [])


if __name__ == '__main__':
    unittest.main()
