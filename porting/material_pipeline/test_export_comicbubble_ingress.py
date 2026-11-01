#!/usr/bin/env python3
"""Bounded ComicBubble source, binding and original-mip rejection coverage."""
import copy
import importlib.util
import json
from pathlib import Path
import struct
import unittest

spec=importlib.util.spec_from_file_location('comicbubble',Path(__file__).with_name('export_comicbubble_ingress.py'))
m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)

@unittest.skipUnless((m.INVENTORY/(m.PACKAGE.replace('/','_')+'.decoded.json')).exists(),'local original evidence unavailable')
class ContractTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.parsed=[m.read_json(m.INVENTORY/((m.PACKAGE+suffix).replace('/','_')+'.decoded.json')) for suffix in ('','_Inst')]
        cls.proofs=[m.read_json(p) for p,_ in m.PROOFS]

    def test_exact_two_owner_contracts_and_complete_graphs(self):
        for instance,parsed in enumerate(self.parsed):
            nodes,roots=m.validate_compiled(parsed,bool(instance))
            self.assertEqual(len(roots['vector_roots']),16 if instance else 38)
            self.assertEqual(len(roots['scalar_roots']),20 if instance else 59)
            self.assertGreater(len(nodes),sum(map(len,roots.values())))
            m.validate_context(self.proofs[instance],bool(instance))

    def test_real_owner_cases_are_separate_from_graph_defaults(self):
        for instance,parsed in enumerate(self.parsed):
            owner=m.read_json(m.INVENTORY/((m.PACKAGE+('_Inst' if instance else '')).replace('/','_')+'.json'))
            graph=m.numeric_contract(parsed,owner,bool(instance),False)
            actual=m.numeric_contract(parsed,owner,bool(instance))
            self.assertGreater(len(graph['owner_uniform_cases']),10)
            self.assertEqual(len(actual['owner_parameter_expectations']),6 if instance else 0)
            if instance:self.assertNotEqual(graph['owner_uniform_cases'][0]['numeric_sha256'],actual['owner_uniform_cases'][0]['numeric_sha256'])
            for cases in (graph['owner_uniform_cases'],actual['owner_uniform_cases']):
                for case in cases:
                    if case['name'].startswith('unmatched_'):self.assertEqual(case['numeric_sha256'],cases[0]['numeric_sha256'])

    def test_mutated_instance_numeric_override_rejected(self):
        owner=m.read_json(m.INVENTORY/((m.PACKAGE+'_Inst').replace('/','_')+'.json'))
        original=m.read_json(next(m.resolve(r['path']) for r in self.proofs[1]['inputs'] if 'instance-properties' in r['path']))
        m.validate_instance_properties(owner,original)
        owner['instanceParameters'][0]['scalars'][0]['value']=0.75
        with self.assertRaisesRegex(ValueError,'original instance properties'):m.validate_instance_properties(owner,original)

    def test_texture_uniform_identity_and_fields_rejected(self):
        for instance in (0,1):
            for key,value in [('texture_index',3),('sampler_source',1),('layer_index',0),('page_table_layer_index',0),('virtual_texture',True)]:
                parsed=copy.deepcopy(self.parsed[instance]);parsed['uniform_expression_arrays']['texture_2d']['expressions'][0]['texture'][key]=value
                with self.subTest(instance=instance,key=key),self.assertRaisesRegex(ValueError,'regular Texture'):m.validate_compiled(parsed,bool(instance))
            parsed=copy.deepcopy(self.parsed[instance]);parsed['uniform_expression_arrays']['texture_2d']['expressions'][0]['parameter']={'name':'Fake'}
            with self.assertRaisesRegex(ValueError,'regular Texture'):m.validate_compiled(parsed,bool(instance))

    def test_layout_program_and_loose_register_mutations_rejected(self):
        for instance in (0,1):
            for mutate in [lambda d:d['material_shaders']['records'][0]['debug_uniform_buffer_layout']['resource_offsets'].__setitem__(0,0),
                           lambda d:d['material_shaders']['records'].pop(),
                           lambda d:d['material_shaders']['records'][2]['resource']['parameter_map']['srvs'].__setitem__(0,[0,1]),
                           lambda d:d.__setitem__('undecoded_bytes',1),
                           lambda d:d['uniform_expression_arrays']['vector'].__setitem__('count',0)]:
                parsed=copy.deepcopy(self.parsed[instance]);mutate(parsed)
                with self.assertRaises(ValueError):m.validate_compiled(parsed,bool(instance))

    def test_ordered_reference_array_retains_unused_tail_for_instance(self):
        proof=copy.deepcopy(self.proofs[1]);proof['ordered_texture_objects'].pop()
        with self.assertRaisesRegex(ValueError,'ordered original'):m.validate_context(proof,True)
        proof=copy.deepcopy(self.proofs[0]);proof['ordered_texture_objects'].reverse()
        with self.assertRaises(ValueError):m.validate_context(proof)
        proof=copy.deepcopy(self.proofs[1]);proof['instance_layer_entries']=1
        with self.assertRaisesRegex(ValueError,'layer'):m.validate_context(proof,True)

@unittest.skipUnless(Path('/tmp/comicbubble-texture-framing-1/receipt.json').exists(),'local original framing evidence unavailable')
class FrameTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.rows=m.read_json(Path('/tmp/comicbubble-texture-framing-1/receipt.json'))['textures']
        cls.blobs=[[Path(r['path']).read_bytes() for r in row['source_files']] for row in cls.rows]

    def test_all_original_logical_mips_and_signed_offsets(self):
        for row,blobs in zip(self.rows,self.blobs):
            result=m.validate_texture_frame(row,blobs)
            self.assertEqual(result['mips'][-1]['logical_width'],1)
            self.assertEqual(result['mips'][-1]['stored_width'],4)
            self.assertEqual(result['mips'][0]['resolved_file_offset'],0)
            self.assertLess(result['mips'][0]['stored_bulk_offset'],0)
            self.assertIsNone(result['effective_sampler'])

    def test_each_file_payload_mutation_rejected(self):
        for row,blobs in zip(self.rows,self.blobs):
            for file in range(3):
                altered=list(blobs);raw=bytearray(altered[file]);raw[0]^=1;altered[file]=bytes(raw)
                with self.subTest(texture=row['texture_object'],file=file),self.assertRaisesRegex(ValueError,'source SHA'):m.validate_texture_frame(row,altered)

    def test_rehashed_logical_frame_offset_virtual_and_truncation_rejected(self):
        for row,blobs in zip(self.rows,self.blobs):
            inline=next(x['resolved_file_offset'] for x in row['mips'] if x['storage']=='uexp')
            for at,fmt,value in [(row['platform_header_offset'],'<I',128),(inline-8,'<q',0),(row['terminal_offset'],'<i',1)]:
                altered=list(blobs);raw=bytearray(altered[1]);struct.pack_into(fmt,raw,at,value);altered[1]=bytes(raw)
                receipt=copy.deepcopy(row);receipt['source_files'][1]['sha256']=m.digest(altered[1])
                with self.assertRaises(ValueError):m.validate_texture_frame(receipt,altered)
            altered=list(blobs);altered[2]=altered[2][:-1];receipt=copy.deepcopy(row)
            receipt['source_files'][2].update(sha256=m.digest(altered[2]),bytes=len(altered[2]))
            with self.assertRaises(ValueError):m.validate_texture_frame(receipt,altered)

@unittest.skipUnless((m.ROOT/'comicbubble-shaders-1/reflection.json').exists(),'local mapped shader evidence unavailable')
class ShaderTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.reflected={r['shader_index']:r for r in m.read_json(m.ROOT/'comicbubble-shaders-1/reflection.json')['records']}
        cls.parsed=[m.read_json(m.INVENTORY/((m.PACKAGE+s).replace('/','_')+'.decoded.json')) for s in ('','_Inst')]

    def test_actual_material_tables_are_distinct_from_loose_font_maps(self):
        for instance,parsed in enumerate(self.parsed):
            for record in parsed['material_shaders']['records']:
                reflection=self.reflected[record['library_association']['index']]
                contract=m.typed_contract(record,reflection,bool(instance))
                self.assertEqual(len(contract['material_resources']),0 if record['frequency']==0 else 2 if instance else 4)

    def test_missing_or_fabricated_material_descriptor_rejected(self):
        for instance,parsed in enumerate(self.parsed):
            record=parsed['material_shaders']['records'][3]
            reflection=copy.deepcopy(self.reflected[record['library_association']['index']])
            descriptor=next(d for d in reflection['descriptors'] if d['resource_table_entries'])
            descriptor['resource_table_entries'][0]['resource']=4
            with self.assertRaises(ValueError):m.typed_contract(record,reflection,bool(instance))
            reflection=copy.deepcopy(self.reflected[record['library_association']['index']]);reflection['descriptors'].pop()
            with self.assertRaises(ValueError):m.typed_contract(record,reflection,bool(instance))

    def test_view_span_and_loose_element_classification_rejected(self):
        for instance,parsed in enumerate(self.parsed):
            record=parsed['material_shaders']['records'][2]
            reflection=copy.deepcopy(self.reflected[record['library_association']['index']])
            next(d for d in reflection['descriptors'] if d['d3d_register']['kind']=='cbv' and d['d3d_register']['index']==1)['block_span_bytes']=64
            with self.assertRaises(ValueError):m.typed_contract(record,reflection,bool(instance))
            reflection=copy.deepcopy(self.reflected[record['library_association']['index']])
            next(d for d in reflection['descriptors'] if d['d3d_register']['kind']=='srv' and not d['resource_table_entries'])['d3d_register']['index']=9
            with self.assertRaises(ValueError):m.typed_contract(record,reflection,bool(instance))

if __name__=='__main__':unittest.main()
