#!/usr/bin/env python3
"""Fail closed on changes to the source-evidenced Blueprint struct."""
import copy
import importlib.util
import json
import os
from pathlib import Path
import unittest

PORTING=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('gen',PORTING/'gen_native_classes.py')
g=importlib.util.module_from_spec(spec);spec.loader.exec_module(g)

def property_record(index,name,kind,kind_raw,offset,refs=None):
 p=dict(index=index,name=name,kind=kind,kind_raw=kind_raw,offset=offset,property_flags='0x10000000000004',array_dim=1,object_flags='0x45',replicated=False)
 if refs is not None:p['type_refs']=refs
 return p

RECORD=dict(name='ConfirmationDialogAction',size=176,align=8,super_fn=None,struct_flag_names=['Native'],struct_ops=None,properties=[
 property_record(0,'ActionName','Name','0x14',168),property_record(1,'Icon','Struct','0x19',32,{'struct':'0x142fdb590'}),
 property_record(2,'ResultName','Name','0x14',24),property_record(3,'DisplayName','Text','0x1d',0)])

def info(rec=RECORD):return g.TypeInfo('struct','0x142b25040',copy.deepcopy(rec),'FConfirmationDialogAction','/Script/FortniteGame','FConfirmationDialogAction.h')

def gate_for(item):
 tables=g.TypeTables.__new__(g.TypeTables)
 slate=g.TypeInfo('struct','0x142fdb590',{'name':'SlateBrush'},'FSlateBrush','/Script/SlateCore','FSlateBrush.h')
 tables.by_fn={item.fn:item,slate.fn:slate}; engine=g.EngineTypeIndex(os.devnull);engine.index['struct']['FSlateBrush']='Styling/SlateBrush.h'
 resolver=g.Resolver({'0x142fdb590':'FSlateBrush'});gate=g.TypeGate(tables,engine,resolver);resolver.gate=gate;return gate

class BlueprintStructTests(unittest.TestCase):
 def test_exact_source_emits_blueprint_type_and_visible_fields(self):
  item=info();gate=gate_for(item);self.assertIsNone(gate.reason(item));model=gate.model(item)
  self.assertTrue(model['blueprint_type']);self.assertEqual([m['name'] for m in model['members']],['DisplayName','ResultName','Icon','ActionName'])
  header=g.render_struct_header(model,'test');self.assertIn('USTRUCT(BlueprintType)',header);self.assertEqual(header.count('UPROPERTY(BlueprintReadWrite)'),4)
  self.assertEqual(model['used'],{'0x142fdb590':{'Struct'}})
 def test_changed_source_identity_or_field_fails_closed(self):
  mutations=[('size',175),('align',4),('super_fn','0x1')]
  for key,value in mutations:
   with self.subTest(key=key):
    item=info();item.rec[key]=value;self.assertIsNotNone(gate_for(item).reason(item))
  for field,value in [('name','Changed'),('kind','Float'),('kind_raw','0xa'),('offset',40),('property_flags','0x10000000000000'),('array_dim',2),('type_refs',{'struct':'0xdead'})]:
   with self.subTest(field=field):
    item=info();item.rec['properties'][1][field]=value;self.assertIsNotNone(gate_for(item).reason(item))
  for field,value in [('fn','0xdead'),('package','/Script/Other')]:
   with self.subTest(field=field):
    item=info();setattr(item,field,value);self.assertIsNotNone(gate_for(item).reason(item))
 def test_unrelated_struct_keeps_default_representation(self):
  item=info();item.cpp_name='FOther';item.fn='0x123';item.rec['name']='Other'
  gate=gate_for(item);self.assertIsNone(gate.reason(item));header=g.render_struct_header(gate.model(item),'test')
  self.assertIn('USTRUCT()',header);self.assertNotIn('BlueprintReadWrite',header)
 def test_local_source_record_if_available(self):
  path=PORTING/'generated/reflection/struct_blocks.json'
  if not path.exists():self.skipTest('optional original reflection absent')
  records=json.loads(path.read_text())['structs'];rec=next(r for r in records if '0x142b25040' in r.get('construct_fns',[]))
  self.assertTrue(g.observed_confirmation_action(info(rec)))
if __name__=='__main__':unittest.main()
