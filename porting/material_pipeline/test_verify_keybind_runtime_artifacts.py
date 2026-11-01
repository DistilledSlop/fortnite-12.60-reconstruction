#!/usr/bin/env python3
"""Reject contradictory runtime receipts even when their individual hashes match."""
import hashlib
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('verifier', Path(__file__).with_name('verify_keybind_runtime_artifacts.py'))
verifier = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verifier)


class SamplingIntegrity(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        pixels = bytes(64 * 64 * 4)
        (self.root / 'original.rgba').write_bytes(pixels)
        (self.root / 'KeybindTexture.rgba').write_bytes(pixels)
        layout = dict(constant_bytes=176, resource_offsets=[144, 152, 160, 168], resource_types=[5, 8, 8, 8])
        self.fixture = dict(texture_contract=dict(expression_kind='Texture', parameter=None,
            named_overrides_allowed=False, default_texture_path='/Test/Original.Original',
            default=dict(rgba=str(self.root / 'original.rgba'), rgbaSha256=hashlib.sha256(pixels).hexdigest())),
            texture_2d_parameters=[dict(expression_kind='Texture', parameter=None)],
            ordered_texture_references=['/Test/Original.Original'], expected_layout=layout)
        self.meta = dict(texture_path='/Test/Original.Original', width=64, height=64, format='PF_B8G8R8A8',
            srgb=True, mips=1, named_proxy_calls=0, constant_bytes=176,
            layout_hash=verifier.common.uniform_layout_hash(176, layout['resource_offsets'], layout['resource_types']),
            resources=[dict(offset=o, type=t) for o, t in zip(layout['resource_offsets'], layout['resource_types'])],
            rgba_sha256=hashlib.sha256(pixels).hexdigest())
        self.draw = dict(width=64, height=64, format='RGBA8', named_proxy_calls=0,
                         numeric_profile='defaults', checker_changed_pixels=1)
        self.image('Source', pixels)
        self.image('Copy', pixels)
        self.image('Checker', b'\x01' + pixels[1:])

    def image(self, label, data):
        (self.root / ('KeybindDraw-' + label + '.rgba')).write_bytes(data)
        self.draw[label.lower() + '_sha1'] = hashlib.sha1(data).hexdigest()

    def check(self):
        (self.root / 'KeybindTexture.json').write_text(json.dumps(self.meta))
        (self.root / 'KeybindDraw.json').write_text(json.dumps(self.draw))
        return verifier.check_sampling(self.fixture, self.root, 'defaults')

    def test_consistent_control_contract(self):
        self.assertEqual(self.check()['checker_changed_pixels'], 1)

    def test_self_consistent_gpu_hash_cannot_replace_original_pixels(self):
        data = b'\x01' + bytes(64 * 64 * 4 - 1)
        (self.root / 'KeybindTexture.rgba').write_bytes(data)
        self.meta['rgba_sha256'] = hashlib.sha256(data).hexdigest()
        with self.assertRaisesRegex(ValueError, 'original canonical'):
            self.check()

    def test_individually_valid_draw_hashes_do_not_prove_copy_equivalence(self):
        self.image('Copy', b'\x01' + bytes(64 * 64 * 4 - 1))
        with self.assertRaisesRegex(ValueError, 'exact-copy draws differ'):
            self.check()

    def test_claimed_checker_effect_requires_actual_changed_pixels(self):
        self.image('Checker', bytes(64 * 64 * 4))
        with self.assertRaisesRegex(ValueError, 'recorded pixel change'):
            self.check()

    def test_exact_checker_change_count_is_required(self):
        self.draw['checker_changed_pixels'] = 4096
        with self.assertRaisesRegex(ValueError, 'recorded pixel change'):
            self.check()

    def test_named_texture_lookup_and_changed_typed_slot_are_rejected(self):
        self.meta['named_proxy_calls'] = 1
        with self.assertRaisesRegex(ValueError, 'RHI layout or named lookup'):
            self.check()
        self.meta['named_proxy_calls'] = 0
        self.meta['resources'][0]['offset'] = 152
        with self.assertRaisesRegex(ValueError, 'RHI layout or named lookup'):
            self.check()

    def test_combined_diagnostic_cannot_be_reported_as_owner_defaults(self):
        self.draw['numeric_profile'] = 'combined'
        with self.assertRaisesRegex(ValueError, 'profile'):
            self.check()


if __name__ == '__main__':
    unittest.main()
