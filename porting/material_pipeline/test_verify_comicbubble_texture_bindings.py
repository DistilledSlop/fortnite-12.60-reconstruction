#!/usr/bin/env python3
"""Receipt validation tests use synthetic values, never runtime evidence."""
import copy
import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('verifier', Path(__file__).with_name('verify_runtime_artifacts.py'))
v = importlib.util.module_from_spec(spec)
spec.loader.exec_module(v)


class ComicTextureReceiptTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)

    def fixture(self, owner):
        size, offsets, types, indices = v.COMIC_OWNERS[owner]
        textures = []
        for path, width, height, mips, srgb in v.COMIC_TEXTURES:
            name = 'FortniteGame/Content/' + path[len('/Game/'):]
            stem = name.split('.', 1)[0]
            files = []
            for suffix in ('.uasset', '.uexp', '.ubulk'):
                p = Path(self.tmp.name) / (stem + suffix)
                p.parent.mkdir(parents=True, exist_ok=True)
                p.write_bytes(suffix.encode())
                files.append(dict(path=str(p), bytes=p.stat().st_size, sha256=hashlib.sha256(p.read_bytes()).hexdigest()))
            textures.append(dict(texture_object=name, source_platform_data=dict(texture_object=name,
                pixel_format='PF_DXT1', logical_width=width, logical_height=height, mip_count=mips,
                first_mip=0, platform_virtual_flag=False, inspector_metadata=dict(srgb=srgb), source_files=files)))
        uniforms, rows = [], []
        for ui, ti, sampler, tex, pair, sampled in indices:
            path, width, height, mips, srgb = v.COMIC_TEXTURES[ti]
            uniforms.append(dict(expression_kind='Texture', parameter=None, texture_index=ti, sampler_source=sampler,
                texture_layer_index=-1, page_table_layer_index=-1, virtual_texture=False, default_texture_path=path))
            rows.append(dict(uniform_index=ui, texture_index=ti, texture_path=path, sampler_source=sampler,
                texture_resource_index=tex, paired_sampler_resource_index=pair, sampled_sampler_resource_index=sampled,
                gt_indexed_match=True, rt_indexed_match=True, texture_binding_match=True, sampler_binding_match=True,
                nonwhite_texture=True, pixel_format='PF_DXT1', logical_width=width, logical_height=height,
                source_mips=mips, resident_mips=mips, rhi_width=width, rhi_height=height, srgb=srgb))
        fixture = dict(expected_layout=dict(constant_bytes=size, resource_offsets=offsets, resource_types=types),
            texture_contract=dict(expression_kind='Texture', parameter=None, named_overrides_allowed=False, textures=textures),
            texture_2d_parameters=uniforms)
        receipt = dict(schema='fortnite.comicbubble-texture-bindings.v1', owner_path=owner,
            ordered_texture_references=[x[0] for x in v.COMIC_TEXTURES], constant_bytes=size, static_slot=255,
            resource_offsets=offsets, resource_types=types, layout_hash=v.uniform_layout_hash(size, offsets, types),
            uniforms=rows, named_proxy_calls=0, shared_wrap_nonnull=True, shared_clamp_nonnull=True)
        return fixture, receipt

    def test_both_owner_contracts_and_resident_mip_sizes(self):
        for owner in v.COMIC_OWNERS:
            with self.subTest(owner=owner):
                fixture, receipt = self.fixture(owner)
                v.verify_comic_texture_bindings(owner, fixture, receipt)
                row = receipt['uniforms'][0]
                row['resident_mips'] -= 1
                row['rhi_width'] //= 2
                row['rhi_height'] //= 2
                v.verify_comic_texture_bindings(owner, fixture, receipt)
                row['rhi_width'] += 1
                with self.assertRaises(ValueError):
                    v.verify_comic_texture_bindings(owner, fixture, receipt)

    def test_receipt_mutations_are_rejected(self):
        owner = next(iter(v.COMIC_OWNERS))
        fixture, original = self.fixture(owner)
        changes = [
            lambda r: r.update(owner_path='wrong'),
            lambda r: r['ordered_texture_references'].reverse(),
            lambda r: r.update(layout_hash=r['layout_hash'] ^ 1),
            lambda r: r.update(static_slot=0),
            lambda r: r['resource_offsets'].__setitem__(0, 16),
            lambda r: r['resource_types'].__setitem__(0, 8),
            lambda r: r.update(named_proxy_calls=1),
            lambda r: r.update(shared_wrap_nonnull=False),
            lambda r: r.update(shared_clamp_nonnull=False),
            lambda r: r['uniforms'].pop(),
            *[lambda r, key=key: r['uniforms'][0].update({key: False}) for key in
              ('gt_indexed_match', 'rt_indexed_match', 'texture_binding_match', 'sampler_binding_match', 'nonwhite_texture')],
            *[lambda r, key=key: r['uniforms'][0].update({key: -1}) for key in
              ('uniform_index', 'texture_index', 'sampler_source', 'texture_resource_index',
               'paired_sampler_resource_index', 'sampled_sampler_resource_index', 'resident_mips', 'rhi_height')],
            lambda r: r['uniforms'][0].update(texture_path='wrong'),
            lambda r: r['uniforms'][0].update(pixel_format='PF_B8G8R8A8'),
            lambda r: r['uniforms'][0].update(srgb=True),
        ]
        for i, change in enumerate(changes):
            receipt = copy.deepcopy(original)
            change(receipt)
            with self.subTest(mutation=i), self.assertRaises(ValueError):
                v.verify_comic_texture_bindings(owner, fixture, receipt)

    def test_source_contract_and_files_are_checked(self):
        owner = next(iter(v.COMIC_OWNERS))
        fixture, receipt = self.fixture(owner)
        changes = [lambda f: f['texture_2d_parameters'][0].update(sampler_source=0),
                   lambda f: f['texture_contract']['textures'][0]['source_platform_data'].update(mip_count=10),
                   lambda f: f['texture_contract']['textures'][1]['source_platform_data']['inspector_metadata'].update(srgb=True),
                   lambda f: f['expected_layout']['resource_types'].__setitem__(0, 8),
                   lambda f: f['texture_contract']['textures'][0]['source_platform_data']['source_files'][0].update(sha256='0'*64)]
        for i, change in enumerate(changes):
            altered = copy.deepcopy(fixture)
            change(altered)
            with self.subTest(mutation=i), self.assertRaises(ValueError):
                v.verify_comic_texture_bindings(owner, altered, receipt)
        file = Path(fixture['texture_contract']['textures'][0]['source_platform_data']['source_files'][0]['path'])
        file.write_bytes(b'changed')
        with self.assertRaises(ValueError):
            v.verify_comic_texture_bindings(owner, fixture, receipt)


if __name__ == '__main__':
    unittest.main()
