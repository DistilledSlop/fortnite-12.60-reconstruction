"""Actual generator admission/render regression using an in-memory entry overlay.

Run from porting after root installs the policy hooks. Outputs only to this task's
ignored artifact directory and exits after both real models pass finalization.
"""
import importlib.util,json,pathlib,sys,copy
ROOT=pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT))
spec=importlib.util.spec_from_file_location('diagnostic_generator',ROOT/'gen_native_classes.py')
g=importlib.util.module_from_spec(spec);spec.loader.exec_module(g)
original_load=g.load_behavior
proposal=json.loads((ROOT/'generator_policies/base_movement_speed80_runtime/manifest-entries.proposed.json').read_text())
specp=importlib.util.spec_from_file_location('air_policy',ROOT/'generator_policies/base_movement_speed80_runtime/source_policy.py')
p=importlib.util.module_from_spec(specp);specp.loader.exec_module(p)
overrides={(e['class'],e['function']):e for e in proposal}
def overlay(behavior_dir):
 entries=original_load(behavior_dir)
 for e in proposal:
  key=(e['class'],e['function'])
  matches=[x for x in entries if (x['class'],x['function']) == key]
  if matches:
   metadata=[{k:v for k,v in x.items() if k not in ('body','reject')} for x in matches]
   if len(matches) != 1 or metadata[0] != e:
    raise ValueError('proposal collision: ' + repr(key))
   if e['kind'] == 'helper' and (matches[0]['body'],matches[0]['reject']) != g.read_snippet(e):
    raise ValueError('proposal body collision: ' + repr(key))
   continue
  e=dict(e);e['body'],e['reject']=g.read_snippet(e) if e['kind'] == 'helper' else (None,None)
  entries.append(e)
 return entries
g.load_behavior=overlay
original_classes=g._athena_playlist_selection_policy.validate_required_classes
def validate_classes(names):
 original_classes(names);p.validate_required_classes(names)
 try:p.validate_required_classes(set(names)-{'AFortPawn'})
 except ValueError:print('PASS missing family class rejected',flush=True)
 else:raise AssertionError('missing class admitted')
g._athena_playlist_selection_policy.validate_required_classes=validate_classes
integrated=False
finalize_owner=g._movement_max_acceleration_policy
original_apply=finalize_owner.apply
def diagnostic_apply(model):
 return p.apply(original_apply(model))
finalize_owner.apply=diagnostic_apply
original=finalize_owner.finalize
seen=set()
def diagnose(model,header,cpp):
 try:
  header,cpp=original(model,header,cpp)
  if not integrated:header,cpp=p.finalize(model,header,cpp)
 except ValueError:
  print(json.dumps({'name':model['name'],'rejected':model.get('splice',{}).get('rejected',[])},default=str,indent=2))
  raise
 if model['name'] in p.PARENTS:
  selected={k:v for k,v in model.items() if k in ['name','parent','package','members_decl_order','functions','helpers','static_helpers','fields','splice']}
  output=ROOT/'generated/agents/base-movement-admission80'
  (output/('real-model-'+model['name']+'.json')).write_text(json.dumps(selected,default=lambda o:list(o) if isinstance(o,set) else str(o)))
  (output/(model['name']+'.h')).write_text(header)
  (output/(model['name']+'.cpp')).write_text(cpp)
  print('PASS real admitted/rendered family: '+model['name'],flush=True)
  seen.add(model['name'])
  if len(seen)==5:raise SystemExit(0)
 return header,cpp
finalize_owner.finalize=diagnose
r=ROOT/'generated/reflection'; out=ROOT/'generated/agents/base-movement-admission80/diagnostic-generation'
g.main(['--promote','--reflection',str(r/'fortnite_game_reflection.json'),'--name-map',str(r/'class_name_map.json'),'--evidence',str(r/'class_name_map.evidence.json'),'--parent-map',str(r/'class_parent_map.json'),'--class-ref-map',str(r/'class_ref_map.json'),'--class-list',str(ROOT/'native_class_set.txt'),'--engine-classes','/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Engine/Classes','--source-root',str(out/'Source'),'--report-dir',str(out/'reports')])
