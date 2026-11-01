import copy,importlib.util,unittest,os,json
from pathlib import Path
PORTING=Path(os.environ.get('TYPED_EVENT_PORTING',str(Path(__file__).resolve().parents[1])))
sp=importlib.util.spec_from_file_location('g',os.environ.get('TYPED_EVENT_GENERATOR',str(PORTING/'gen_native_classes.py')));g=importlib.util.module_from_spec(sp);sp.loader.exec_module(g)
EXPECTED={
'OnActionProgress':['float HeldPercent'],
'OnTriggeredInputActionChanged':['const FDataTableRowHandle& NewTriggeredAction'],
'ShowNotice':['const FText& Title','const FText& Body'],
'ConfigureConfirmationButton':['UCommonButton* Button','const FConfirmationDialogAction& Action','bool bSimpleConfirm'],
'ConfigureDeclineButton':['UCommonButton* Button'],
'OnOverrideCloseButtonText':['const FText& OverrideText'],
'UpdateBorderWidth':['float HalfBorderWidth'],
'OnButtonSet':['bool InbShouldExit','bool bShouldShow'],
'OnDisplayToast':['UFortUINotification* NewToast'],
'OnInputModeChanged':['ECommonInputType NewInputType'],
'DisplayWidget':['UWidget* WebWidget'],
}
class R(g.Resolver):
 def __init__(self):super().__init__({'0x1446f2880':'FDataTableRowHandle','0x142b25040':'FConfirmationDialogAction','0x140807800':'UCommonButton','0x142b26c60':'UFortUINotification','0x14081ee20':'ECommonInputType','0x143dc3650':'UWidget'})
class TypedEvents(unittest.TestCase):
 def build(self,row,fn=None,owners=None):
  owner,name,outer,create,flags,access,params,_,_,_=row
  f=fn or dict(name=name,create_func=create,flags=[hex(flags)],outer_fn=outer,params=copy.deepcopy(params));r=R()
  return g.build_functions_promoted([f],r,set(),owners or {outer},parameterized_blueprint_events=g.UI_PARAMETERIZED_BLUEPRINT_EVENTS[owner]),r
 def test_all_events_and_exact_mutation_guards(self):
  self.assertEqual(len(g.OBSERVED_TYPED_SUPER_EVENTS),11)
  for row in g.OBSERVED_TYPED_SUPER_EVENTS:
   with self.subTest(event=row[1]):
    owner,name,outer,create,flags,access,params,_,_,_=row;out,r=self.build(row);self.assertEqual(out[0]['args'],EXPECTED[name]);self.assertEqual(out[0]['access'],access);self.assertFalse(r.skipped)
    self.assertFalse(self.build(row,owners={'0xdead'})[0]);f=dict(name=name,create_func='0xdead',flags=[hex(flags)],outer_fn=outer,params=copy.deepcopy(params));self.assertFalse(self.build(row,f)[0])
    f['create_func']=create;f['flags']=[hex(flags|0x400)];self.assertFalse(self.build(row,f)[0]);f['flags']=[hex(flags)];f['params'][0]['property_flags']='0x0';self.assertFalse(self.build(row,f)[0])
 def test_missing_type_fails_closed(self):
  row=next(z for z in g.OBSERVED_TYPED_SUPER_EVENTS if z[1]=='OnInputModeChanged');owner,name,outer,create,flags,access,params,*_=row;r=R();r.types={};f=dict(name=name,create_func=create,flags=[hex(flags)],outer_fn=outer,params=params)
  self.assertFalse(g.build_functions_promoted([f],r,set(),{outer},parameterized_blueprint_events=g.UI_PARAMETERIZED_BLUEPRINT_EVENTS[owner]))
 def test_real_reflection_and_scoped_type_gate_if_available(self):
  folder=PORTING/'generated/reflection'; engine=PORTING.parents[1]/'UnrealEngine/Engine'
  required=['fortnite_game_reflection.json','class_name_map.json','class_name_map.evidence.json','class_ref_map.json','struct_blocks.json','enum_blocks.json','delegate_blocks.json','construct_fn_to_block.json']
  if not engine.is_dir() or not all((folder/n).is_file() for n in required): self.skipTest('optional original reflection and Engine sources unavailable')
  def load(n):return json.loads((folder/n).read_text())
  names=load('class_name_map.json'); ev=load('class_name_map.evidence.json'); refs=load('class_ref_map.json')['resolved'];types={}
  for block in ev['blocks']:
   name=names.get(block['block_va'])
   if name:
    for fn in block.get('construct_fns',[]):types['0x'+fn.removeprefix('0x')]=name
  for fn,block in refs.items():
   if block in names:types[fn]=names[block]
  tables=g.TypeTables(str(folder));types.update({fn:info.cpp_name for fn,info in tables.by_fn.items()})
  resolver=g.Resolver(types);gate=g.TypeGate(tables,g.EngineTypeIndex(str(engine)),resolver);resolver.gate=gate
  data=load('fortnite_game_reflection.json'); funcs={f.get('create_func'):f for b in data['classes_in_groups_with_fort_names'] for f in b.get('functions',[])}
  for row in g.OBSERVED_TYPED_SUPER_EVENTS:
   owner,name,outer,create,flags,access,params,expected_types,args,allowed=row
   gate.current_package='/Script/CommonUI' if owner=='UCommonButton' else '/Script/FortniteUI';resolver.skipped=[];resolver.tokens=[]
   out=g.build_functions_promoted([funcs[create]],resolver,set(),{outer},parameterized_blueprint_events=g.UI_PARAMETERIZED_BLUEPRINT_EVENTS[owner])
   self.assertEqual(out[0]['args'],EXPECTED[name]);self.assertFalse(resolver.skipped);self.assertFalse(getattr(resolver,'observed_event_type_packages',{}))
  for name,fn in [('OnInputModeChanged','0x14081ee20'),('ConfigureConfirmationButton','0x142b25040')]:
   row=next(z for z in g.OBSERVED_TYPED_SUPER_EVENTS if z[1]==name);owner,_,outer,create,_,_,params,*_=row
   info=gate.info(fn);old=info.package;gate.current_package='/Script/FortniteUI'
   try:
    info.package='/Script/Unrelated';resolver.skipped=[];resolver.tokens=[]
    out=g.build_functions_promoted([funcs[create]],resolver,set(),{outer},parameterized_blueprint_events=g.UI_PARAMETERIZED_BLUEPRINT_EVENTS[owner])
    self.assertFalse(out);self.assertFalse(getattr(resolver,'observed_event_type_packages',{}))
   finally:info.package=old
   entry=next(p for p in params if fn in (p.get('type_refs') or {}).values());resolver.tokens=[];resolver.ctype(entry,{})
   self.assertTrue(resolver.tokens);self.assertIn('OtherModule',resolver.tokens[-1])
if __name__=='__main__':unittest.main()
