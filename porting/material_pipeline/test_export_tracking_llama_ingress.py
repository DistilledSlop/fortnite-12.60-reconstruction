#!/usr/bin/env python3
"""Source-authenticity and rejection controls for TrackingLlama ingress."""
import copy
import importlib.util
import json
from pathlib import Path
import unittest

HERE=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('tracking_ingress',HERE/'export_tracking_llama_ingress.py')
export=importlib.util.module_from_spec(spec); spec.loader.exec_module(export)

class TrackingIngressTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.checkpoint=export.create_checkpoint()
        cls.owner=cls.checkpoint['owners'][0]
        cls.frame=export.read_json(Path('/tmp/tracking-llama-texture-frame-1/receipt.json'))['textures'][0]
        cls.blobs=[Path(x['path']).read_bytes() for x in cls.frame['source_files']]

    def test_complete_source(self):
        self.assertEqual(len(self.owner['original_shader_code']),19)
        self.assertEqual(len(self.owner['slate_programs']),7)
        self.assertEqual(len(self.checkpoint['textures'][0]['mips']),9)
        self.assertEqual(self.owner['numeric_contract']['numeric_comparison_bytes'],104)
        self.assertEqual(self.owner['texture_context_proof']['ordered_texture_objects'],[export.TEXTURE])

    def test_reject_original_contract_mutations(self):
        for mutate in ('texture','layout','index','count'):
            data=copy.deepcopy(self.owner['decoded'])
            if mutate=='texture': data['uniform_expression_arrays']['texture_2d']['expressions'][0]['texture']['texture_index']=1
            if mutate=='layout': data['material_shaders']['records'][2]['debug_uniform_buffer_layout']['resource_offsets'][0]+=8
            if mutate=='index': data['material_shaders']['records'][2]['library_association']['index']+=1
            if mutate=='count': data['shader_record_count']=18
            with self.subTest(mutate=mutate), self.assertRaises(ValueError): export.validate_compiled(data)

    def test_shader_binding_contract_and_rejection(self):
        directory=export.ROOT/'tracking-llama-shaders-1'
        reflection=export.read_json(directory/'reflection.json')['records'][2]
        record=self.owner['slate_programs'][2]
        contract=export.typed_contract(record,reflection)
        self.assertFalse(contract['requires_view'])
        changed=copy.deepcopy(reflection)
        descriptor=next(x for x in changed['descriptors'] if x['resource_table_entries'])
        descriptor['resource_table_entries'][0]['resource']=3
        with self.assertRaises(ValueError): export.typed_contract(record,changed)

    def test_reject_payload_and_receipt_mutations(self):
        blobs=self.blobs.copy(); blobs[2]=bytes([blobs[2][0]^1])+blobs[2][1:]
        with self.assertRaises(ValueError): export.validate_texture_frame(self.frame,blobs)
        row=copy.deepcopy(self.frame); row['mips'][0]['stored_bulk_offset']+=1
        with self.assertRaises(ValueError): export.validate_texture_frame(row,self.blobs)

if __name__=='__main__': unittest.main()
