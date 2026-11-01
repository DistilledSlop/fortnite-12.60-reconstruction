import copy,importlib.util,json,unittest
from pathlib import Path
P=Path(__file__).resolve().parent

def module(name):
 s=importlib.util.spec_from_file_location(name,P/(name+'.py'));m=importlib.util.module_from_spec(s);s.loader.exec_module(m);return m
producer=module('export_google_icon_ingress');frame=module('google_frame')
R=producer.ROOT;I=producer.GENERATED/'frontend-probes/frontend-material-inventory';stem=producer.PACKAGE.replace('/','_')
def doc(p):return producer.read_json(p)[0]
class SourceProofTests(unittest.TestCase):
 @classmethod
 def setUpClass(c):
  c.owner=doc(I/(stem+'.json'));c.exports=doc(I/(stem+'.export-map.json'));c.parsed=doc(I/(stem+'.decoded.json'));c.fixture=doc(R/'google-ingress-candidate-1/runtime-fixture.json')
 def test_exact_context_and_no_function_gap(self):
  p=producer.prove_context(self.owner,self.exports);self.assertEqual(p['ordered_texture_references'],[producer.TEXTURE_PATH]);self.assertEqual(p['trace'][0]['expression_position'],0)
 def test_wrong_named_parameter_rejected(self):
  d=copy.deepcopy(self.owner);d['expressionExports'][1]['properties'][0]['value']='NameTexture (NameProperty)'
  with self.assertRaises(ValueError):producer.prove_context(d,self.exports)
 def test_extra_expression_and_wrong_export_index_rejected(self):
  for mode in range(2):
   d=copy.deepcopy(self.owner)
   if mode:d['materialProperties'][0]['expressionRefs'][0]['index']=2
   else:d['materialProperties'][0]['expressionRefs'][2]=dict(name='MaterialExpressionTextureSampleParameter2D_0',index=3,isImport=False)
   with self.assertRaises(ValueError):producer.prove_context(d,self.exports)
 def test_original_numeric_defaults_not_editor_iconcolor(self):
  nodes,roots=producer.validate_arrays(self.parsed);cases=producer.uniform_cases(self.parsed,nodes)
  self.assertEqual(len(cases),8);self.assertEqual(cases[0]['numeric_values'][:9],[0.0]*9);self.assertAlmostEqual(cases[0]['numeric_values'][9],0.2)
  self.assertEqual(cases[0]['numeric_values'][10],0.0);self.assertEqual({n['parameter']['name']['name'] for n in nodes if 'parameter'in n},{'SelectionColor','IconFilter','RefractionDepthBias'})
 def test_sampler3_and_named_identity_mismatch_rejected(self):
  for field,value in [('sampler_source',2),('texture_index',1)]:
   d=copy.deepcopy(self.parsed);d['uniform_expression_arrays']['texture_2d']['expressions'][0]['texture'][field]=value
   with self.assertRaises(ValueError):producer.validate_arrays(d)
 def test_full_source_mips_and_cue_fixup(self):
  meta=doc(R/'google-texture-1/texture-report.json');files=[(producer.EXTRACTED/(producer.TEXTURE_PACKAGE+s)).read_bytes() for s in ['.uasset','.uexp','.ubulk']];d=frame.frame(*files,meta)
  self.assertEqual((d['external_mips'],d['inline_mips']),(3,7));self.assertEqual(d['mip_receipts'][0]['stored_bulk_offset'],-4036);self.assertEqual(d['mip_receipts'][9]['logical_width'],1);self.assertEqual(d['mip_receipts'][9]['stored_width'],4)
  bad=copy.deepcopy(meta);bad['mips'][3]['bulkOffset']=1052
  with self.assertRaises(ValueError):frame.frame(*files,bad)
 def test_external_bulk_corruption_and_truncation_rejected(self):
  meta=doc(R/'google-texture-1/texture-report.json');files=[(producer.EXTRACTED/(producer.TEXTURE_PACKAGE+s)).read_bytes() for s in ['.uasset','.uexp','.ubulk']]
  for bulk in [files[2][:-1],bytes([files[2][0]^1])+files[2][1:]]:
   with self.assertRaises(ValueError):frame.frame(files[0],files[1],bulk,meta)
 def test_asset_sampler_typed_resource_link_rejected(self):
  f=self.fixture['shaders'][2];producer.typed_contract(f['record'],f['reflection']);bad=copy.deepcopy(f['reflection'])
  for d in bad['descriptors']:
   if d['d3d_register']['kind']=='sampler' and d['resource_table_entries']:d['resource_table_entries'][0]['resource']=3
  with self.assertRaises(ValueError):producer.typed_contract(f['record'],bad)
if __name__=='__main__':unittest.main()
