import importlib.util
from pathlib import Path
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location('completion', Path(__file__).with_name('inspect_gi_update_completion.py'))
M = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(M)


class CompletionTests(unittest.TestCase):
    def test_wrong_or_reversed_event_offsets_rejected(self):
        assembly = ('FortUpdateStatusControl:\n leaq 40(%rdi), %rax\n retq\n.Lfunc_end1:\n'
                    'FortUpdateCompleteControl:\n leaq 64(%rdi), %rax\n retq\n.Lfunc_end2:\n')
        M.assembly_offsets(assembly)
        for bad in (assembly.replace('64(%rdi)', '40(%rdi)'),
                    assembly.replace('40(%rdi)', '64(%rdi)'),
                    assembly.replace('retq', 'callq injected', 1)):
            with self.assertRaises(ValueError):
                M.assembly_offsets(bad)

    def test_unpinned_input_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'image'
            path.write_bytes(b'unreviewed')
            with self.assertRaisesRegex(ValueError, 'input hash'):
                M.pinned(path, M.IMAGE_SHA)

    def test_wrong_opcode_cannot_supply_same_displacement(self):
        data = bytearray(16)
        data[:5] = bytes.fromhex('e8fb0f0000')
        self.assertEqual(M.displacement(data, M.BASE, b'\xe8', 5), M.BASE + 0x1000)
        with self.assertRaises(ValueError):
            M.displacement(data, M.BASE, b'\xe9', 5)


if __name__ == '__main__':
    unittest.main()
