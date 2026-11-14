"""Actual generator admission/render regression using an in-memory entry overlay.

Run from porting after root installs the policy hooks. Outputs only to this task's
ignored artifact directory and exits after all three real models pass finalization.
"""
import importlib.util,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT))
spec=importlib.util.spec_from_file_location('diagnostic_generator',ROOT/'gen_native_classes.py')
g=importlib.util.module_from_spec(spec);spec.loader.exec_module(g)
original_load=g.load_behavior
proposal=json.loads((ROOT/'generator_policies/athena_playlist_selection_runtime/manifest-entries.json').read_text())
overrides={(e['class'],e['function']):e for e in proposal}
def overlay(behavior_dir):
 entries=original_load(behavior_dir)
 for i,e in enumerate(entries):
  key=(e['class'],e['function'])
  if key in overrides:
   e=dict(overrides[key]);e['body'],e['reject']=g.read_snippet(e);entries[i]=e
 return entries
g.load_behavior=overlay
original=g._athena_playlist_selection_policy.finalize
seen=set()
def diagnose(model,header,cpp):
 try:
  header,cpp=original(model,header,cpp)
 except ValueError:
  print(json.dumps({'name':model['name'],'rejected':model.get('splice',{}).get('rejected',[])},default=str,indent=2))
  raise
 if model['name'] in g._athena_playlist_selection_policy.DECLARATIONS:
  selected={k:v for k,v in model.items() if k in ['name','parent','package','members_decl_order','functions','helpers','static_helpers','splice']}
  output=ROOT/'generated/agents/athena-playlist-setters5'
  (output/('real-model-'+model['name']+'.json')).write_text(json.dumps(selected,default=lambda o:list(o) if isinstance(o,set) else str(o)))
  (output/(model['name']+'.h')).write_text(header)
  (output/(model['name']+'.cpp')).write_text(cpp)
  print('PASS real admitted/rendered family: '+model['name'],flush=True)
  seen.add(model['name'])
  if len(seen)==3:raise SystemExit(0)
 return header,cpp
g._athena_playlist_selection_policy.finalize=diagnose
r=ROOT/'generated/reflection'; out=ROOT/'generated/agents/athena-playlist-setters5/diagnostic-generation'
g.main(['--promote','--reflection',str(r/'fortnite_game_reflection.json'),'--name-map',str(r/'class_name_map.json'),'--evidence',str(r/'class_name_map.evidence.json'),'--parent-map',str(r/'class_parent_map.json'),'--class-ref-map',str(r/'class_ref_map.json'),'--class-list',str(ROOT/'native_class_set.txt'),'--engine-classes','/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Engine/Classes','--source-root',str(out/'Source'),'--report-dir',str(out/'reports')])
