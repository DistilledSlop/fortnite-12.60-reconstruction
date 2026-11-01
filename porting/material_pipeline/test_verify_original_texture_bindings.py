"""Synthetic receipt tests; these fixtures are never used as runtime evidence."""
import copy
import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('verifier', Path(__file__).with_name('verify_runtime_artifacts.py'))
v = importlib.util.module_from_spec(spec)
spec.loader.exec_module(v)


class OriginalTextureBindingTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)

    def fixture(self, indices=(0,), samplers=(0,), fmt='PF_DXT5'):
        owner = '/Game/Unit/Material.Material'
        refs = ['/Game/Unit/Texture%d.Texture%d' % (i, i) for i in range(max(indices) + 1)]
        textures = []
        for i, path in enumerate(refs):
            source_path = v.original_textures.source_object(path)
            data = bytes([i + 1]) * (8 if fmt == 'PF_DXT1' else 16)
            files = []
            for suffix, raw in (('.uasset', b'synthetic-header'), ('.uexp', data)):
                p = Path(self.tmp.name) / (source_path.split('.')[0] + suffix)
                p.parent.mkdir(parents=True, exist_ok=True)
                p.write_bytes(raw)
                files.append(dict(path=str(p), bytes=len(raw), sha256=hashlib.sha256(raw).hexdigest()))
            mip = dict(level=0, logical_width=4, logical_height=4, stored_width=4, stored_height=4,
                compressed_bytes=len(data), storage='uexp', resolved_file_offset=0,
                sha256=hashlib.sha256(data).hexdigest())
            textures.append(dict(texture_object=source_path, source_platform_data=dict(texture_object=source_path,
                logical_width=4, logical_height=4, mip_count=1, pixel_format=fmt, first_mip=0,
                platform_virtual_flag=False, source_files=files, inspector_metadata=dict(srgb=True), mips=[mip]),
                mips=[dict(**mip, compressed_hex=data.hex())]))
        types = [5, 8] * len(indices) + [8, 8]
        offsets = [32 + 8 * i for i in range(len(types))]
        layout = dict(constant_bytes=32 + 8 * len(types), resource_offsets=offsets, resource_types=types)
        entries, bindings, rows = [], [], []
        for i, (index, sampler) in enumerate(zip(indices, samplers)):
            entries.append(dict(expression_kind='Texture', parameter=None, texture_index=index, sampler_source=sampler,
                texture_layer_index=-1, page_table_layer_index=-1, virtual_texture=False, default_texture_path=refs[index]))
            bindings.append(dict(uniform_position=i, texture_index=index, sampler_source=sampler, layer_index=-1,
                page_table_layer_index=-1, virtual_texture=False, texture_object=v.original_textures.source_object(refs[index])))
            rows.append(dict(uniform_index=i, texture_index=index, sampler_source=sampler, texture_path=refs[index],
                texture_resource_index=2 * i, paired_sampler_resource_index=2 * i + 1,
                sampled_sampler_resource_index=2 * i + 1 if sampler == 0 else 2 * len(indices) + sampler - 1,
                gt_indexed_match=True, rt_indexed_match=True, texture_binding_match=True, sampler_binding_match=True,
                nonwhite_texture=True, pixel_format=fmt, logical_width=4, logical_height=4,
                source_mips=1, resident_mips=1, rhi_width=4, rhi_height=4, srgb=True))
        f = dict(material=dict(material_path=owner), ordered_texture_references=refs, vector_roots=[0], scalar_roots=[1],
            expected_layout=layout, texture_2d_parameters=entries, provenance=dict(resource_sha256='aa' * 32),
            texture_contract=dict(expression_kind='Texture', parameter=None, named_overrides_allowed=False, textures=textures),
            texture_context_proof=dict(schema='fortnite.original-material-texture-context-proof.v1',
                owner='FortniteGame/Content/Unit/Material', original_resource_sha256='aa' * 32,
                ordered_texture_objects=[v.original_textures.source_object(p) for p in refs], texture_2d_bindings=bindings))
        h = v.uniform_layout_hash(layout['constant_bytes'], offsets, types)
        r = dict(schema='fortnite.original-texture-bindings.v1', owner_path=owner, ordered_texture_references=refs,
            static_slot=255, layout_hash=h, **layout, named_proxy_calls=0,
            shared_wrap_nonnull=True, shared_clamp_nonnull=True, uniforms=rows)
        return owner, f, r, h

    def test_asset_wrap_clamp_and_reversed_indices(self):
        for indices, samplers in (((0,), (0,)), ((0,), (1,)), ((0,), (2,)), ((1, 0), (2, 0))):
            for fmt in ('PF_DXT1', 'PF_DXT3', 'PF_DXT5'):
                v.original_textures.verify(*self.fixture(indices, samplers, fmt))

    def test_binding_and_reference_mutations_reject(self):
        owner, f, original, h = self.fixture((1, 0), (2, 0))
        mutations = [lambda r: r['ordered_texture_references'].reverse(), lambda r: r.update(layout_hash=h ^ 1),
            lambda r: r.update(named_proxy_calls=False), lambda r: r.update(shared_clamp_nonnull=False),
            lambda r: r['uniforms'].pop(), lambda r: r['resource_offsets'].__setitem__(0, 0)]
        mutations += [lambda r, k=k: r['uniforms'][0].update({k: False}) for k in
            ('gt_indexed_match', 'rt_indexed_match', 'texture_binding_match', 'sampler_binding_match', 'nonwhite_texture')]
        mutations += [lambda r, k=k: r['uniforms'][0].update({k: -1}) for k in
            ('texture_index', 'texture_resource_index', 'sampled_sampler_resource_index', 'resident_mips', 'rhi_width')]
        for change in mutations:
            r = copy.deepcopy(original); change(r)
            with self.assertRaises(ValueError): v.original_textures.verify(owner, f, r, h)

    def test_source_graph_and_compressed_bytes_reject(self):
        owner, original, r, h = self.fixture()
        mutations = [lambda f: f['texture_context_proof'].update(owner='wrong'),
            lambda f: f['provenance'].update(resource_sha256='bb' * 32),
            lambda f: f['texture_2d_parameters'][0].update(texture_index=False),
            lambda f: f['texture_2d_parameters'][0].update(sampler_source=3),
            lambda f: f['texture_contract']['textures'][0]['mips'][0].update(compressed_hex='00' * 16),
            lambda f: f['texture_contract']['textures'][0]['source_platform_data']['mips'][0].update(resolved_file_offset=1),
            lambda f: f['texture_contract']['textures'][0]['source_platform_data']['source_files'][0].update(sha256='0' * 64)]
        for change in mutations:
            f = copy.deepcopy(original); change(f)
            with self.assertRaises(ValueError): v.original_textures.verify(owner, f, r, h)


if __name__ == '__main__':
    unittest.main()
