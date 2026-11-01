"""Reject changes to the original per-vector zero/call/copy contract."""
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('caller', Path(__file__).with_name('inspect_texture_property_caller.py'))
caller = importlib.util.module_from_spec(spec)
spec.loader.exec_module(caller)


class CallerContract(unittest.TestCase):
    def test_original_loop_covers_all_four_lanes_each_iteration(self):
        result = caller.verify_vector_loop(caller.LOOP)
        self.assertEqual(result['call_slot'], 0x18)
        self.assertEqual(result['result_copy_bytes'], 16)
        self.assertEqual(result['output_stride_bytes'], 16)

    def test_missing_zero_wrong_slot_partial_copy_and_missing_backedge_rejected(self):
        for before, after in [(b'\x0f\x57\xc0', b'\x90\x90\x90'),
                              (b'\xff\x50\x18', b'\xff\x50\x28'),
                              (b'\x0f\x11\x07', b'\xf3\x0f\x11'),
                              (b'\x7c\xce', b'\x90\x90')]:
            with self.subTest(instruction=before.hex()), self.assertRaises(ValueError):
                caller.verify_vector_loop(caller.LOOP.replace(before, after))

    def test_actual_original_image_when_available(self):
        import mmap
        path = caller.GENERATED / 'dump/FortniteClient-Win64-Shipping.runtime.bin'
        if not path.is_file():
            self.skipTest('original runtime memory image unavailable')
        with path.open('rb') as handle, mmap.mmap(handle.fileno(), 0, access=mmap.ACCESS_READ) as image:
            result, windows = caller.inspect(image)
        self.assertEqual(result['entry'], '0x144353900')
        self.assertEqual(result['direct_size_property_success_lanes'][2:], ['+0.0', '+0.0'])
        self.assertEqual(len(windows['vector-loop.bin']), 50)


if __name__ == '__main__':
    unittest.main()
