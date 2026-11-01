#!/usr/bin/env python3
"""Guard exact shader instruction comparison and native payload boundaries."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('verifier', Path(__file__).with_name('verify_runtime_artifacts.py'))
verifier = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verifier)


def module(name, literal=17):
    raw = name.encode() + b'\0'
    raw += b'\0' * (-len(raw) % 4)
    text = list(struct.unpack('<' + 'I' * (len(raw) // 4), raw))
    words = [0x07230203, 0x10000, 0, 64, 0]
    words += [(len(text) + 4) << 16 | 15, 0, 2, *text, 11]
    words += [4 << 16 | 43, 1, 12, literal]
    return struct.pack('<' + 'I' * len(words), *words)


class RuntimeArtifactVerificationTests(unittest.TestCase):
    def test_texture_links_use_material_resource_slots_and_exclude_font_globals(self):
        def descriptor(kind, table, resource, binding, vk_type):
            return dict(binding=binding, vulkan_descriptor_type=vk_type,
                        d3d_register=dict(kind=kind, index=0), resource_table_entries=[
                            dict(uniform_buffer=1, resource=resource, bind=0, table=table)])
        record=dict(material_uniform_buffer=dict(base_index=1))
        shader=dict(reflection=dict(descriptors=[descriptor('srv','texture',0,3,2),
            descriptor('sampler','sampler',1,2,0), dict(binding=5, resource_table_entries=[])]))
        layout=dict(resource_types=[5,8,8,8])
        header=dict(globals=[dict(binding=3,type_index=0),dict(binding=2,type_index=1),dict(binding=5,type_index=2)],
                    global_types=[2,0,2], uniform_buffers=[dict(verified_role='Material',resources=[[0,0,0,5],[1,0,1,8]])])
        verifier.check_material_resource_links(record,shader,layout,header)
        header['uniform_buffers'][0]['resources'].append([2,1,2,5])
        with self.assertRaisesRegex(ValueError,'resource-table links'):
            verifier.check_material_resource_links(record,shader,layout,header)
        header['uniform_buffers'][0]['resources']=[[1,0,0,5],[0,0,1,8]]
        with self.assertRaisesRegex(ValueError,'resource-table links'):
            verifier.check_material_resource_links(record,shader,layout,header)

    def test_rhi_hash_preserves_reverse_type_packing_and_layout_bounds(self):
        self.assertEqual(verifier.uniform_layout_hash(336, [320, 328], [8, 8]), 22544639)
        self.assertEqual(verifier.uniform_layout_hash(208, [192, 200], [8, 8]), 14156031)
        self.assertEqual(verifier.uniform_layout_hash(32, [16, 24], [5, 8]), 0x002500ff)
        self.assertEqual(verifier.uniform_layout_hash(48, [16, 24, 32, 40], [5, 8, 6, 7]), 0x053806f8)
        for offsets, types in [([17], [8]), ([32], [8]), ([16, 24], [8])]:
            with self.assertRaisesRegex(ValueError, 'layout shape'):
                verifier.uniform_layout_hash(32, offsets, types)

    def test_original_typed_registers_distinguish_view_and_material(self):
        record = dict(material_uniform_buffer=dict(base_index=2, bound=True), uniform_parameters=[
            dict(struct_name=dict(name='FViewUniformShaderParameters'), binding=dict(base_index=1, bound=True))])
        shader = dict(reflection=dict(descriptors=[
            dict(binding=9, d3d_register=dict(kind='cbv', space=0, index=2)),
            dict(binding=4, d3d_register=dict(kind='cbv', space=0, index=1))]))
        self.assertEqual(verifier.expected_uniform_bindings(record, shader, 111, 222),
                         {9: ('Material', 111), 4: ('View', 222)})
        with self.assertRaisesRegex(ValueError, 'actual current metadata'):
            verifier.expected_uniform_bindings(record, shader, 111, None)

    def test_trimmed_material_binding_uses_original_register_and_layout(self):
        record = dict(material_uniform_buffer=dict(base_index=1, bound=True), uniform_parameters=[])
        descriptor = dict(binding=4, d3d_register=dict(kind='cbv', space=0, index=1), block_span_bytes=608)
        shader = dict(reflection=dict(descriptors=[descriptor]))
        layout = dict(resource_offsets=[736, 744], resource_types=[8, 8])
        for span in (608, 720, 736):
            descriptor['block_span_bytes'] = span
            self.assertEqual(verifier.expected_uniform_bindings(record, shader, 111, None, layout), {4: ('Material', 111)})
        for span in (0, 607, 752):
            descriptor['block_span_bytes'] = span
            with self.assertRaisesRegex(ValueError, 'span'):
                verifier.expected_uniform_bindings(record, shader, 111, None, layout)
        descriptor['block_span_bytes'] = 608
        descriptor['d3d_register']['index'] = 2
        with self.assertRaisesRegex(ValueError, 'roles'):
            verifier.expected_uniform_bindings(record, shader, 111, None, layout)
        descriptor['d3d_register']['index'] = 1
        record['material_uniform_buffer']['base_index'] = 2
        with self.assertRaisesRegex(ValueError, 'roles'):
            verifier.expected_uniform_bindings(record, shader, 111, None, layout)

    def test_name_only_adaptation_preserves_interfaces_and_other_instructions(self):
        original = verifier.normalize_entrypoint(module('main'))
        changed_name = verifier.normalize_entrypoint(module('main_00000000_12345678'))
        self.assertEqual(original[:2], changed_name[:2])
        self.assertNotEqual(original[2], changed_name[2])
        changed_literal = verifier.normalize_entrypoint(module('main_00000000_12345678', 18))
        self.assertNotEqual(original[:2], changed_literal[:2])

    def test_native_payload_requires_exact_spirv_and_optional_trailer_bounds(self):
        words = [0x07230203, 0x10000, 0, 64, 0]
        # Eight empty metadata arrays, flags/source/CRC, two offset arrays,
        # empty FString, then the SPIR-V TArray and FShaderCode trailer.
        header = bytes(8 * 4) + struct.pack('<II20sI', 3, 1, bytes(20), 0)
        header += bytes(3 * 4)
        payload = header + struct.pack('<i5II', 5, *words, 4)
        info, code = verifier.read_payload(payload)
        self.assertEqual(info['inout_mask'], 3)
        self.assertEqual(code, struct.pack('<5I', *words))
        for bad in (payload[:-1], payload + b'\0', payload[:-4] + struct.pack('<I', 8)):
            with self.assertRaises(ValueError):
                verifier.read_payload(bad)

    def test_instruction_cannot_overrun_module(self):
        with self.assertRaisesRegex(ValueError, 'instruction'):
            verifier.normalize_entrypoint(module('main')[:-4])


if __name__ == '__main__':
    unittest.main()
