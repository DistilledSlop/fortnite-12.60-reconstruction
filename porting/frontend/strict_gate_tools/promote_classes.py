from pathlib import Path
import hashlib,json,os,subprocess,sys
r=Path('/opt/fortnite/FortniteClient-12.60-CL-13477524');root=r/'porting/native/Source';out=Path('/tmp/frontend-strict-promotion-15');out.mkdir(exist_ok=False)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
import importlib.util
for folder,filename,method in (
 ('common_lazy_image_connected_runtime','policy.py','validate_source'),
 ('common_animated_switcher_runtime','policy.py','validate_source'),
 ('common_queue_widget_lifecycle_runtime','policy.py','validate_source'),
 ('strict_commonui_imports_runtime','policy.py','validate_original_image'),
 ('client_event_dispatch_runtime','policy.py','validate_source'),
 ('ui_config_singleton_runtime','policy.py','validate_source'),
 ('legacy_state_event_bridge_runtime','policy.py','validate_source'),
 ('activatable_route_bridge_runtime','policy.py','validate_source'),
 ('common_action_route_connected_runtime','policy.py','validate_source'),
 ('common_button_metadata_runtime','policy.py','validate_source'),
 ('common_input_modality_runtime','policy.py','validate_source'),
 ('common_analog_cursor_runtime','settings_policy.py','validate_source')):
 spec=importlib.util.spec_from_file_location(folder,r/'porting/frontend'/folder/filename);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 getattr(m,method)('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
print('connected loading/routing/modality bounded source gates PASS',flush=True)
before={str(p.relative_to(root)):dict(sha256=sha(p),atime=p.stat().st_atime_ns,mtime=p.stat().st_mtime_ns) for p in root.rglob('*') if p.is_file()}
args=[sys.executable,'-I','porting/gen_native_classes.py','--promote','--reflection','porting/generated/reflection/fortnite_game_reflection.json','--name-map','porting/generated/reflection/class_name_map.json','--evidence','porting/generated/reflection/class_name_map.evidence.json','--parent-map','porting/generated/reflection/class_parent_map.json','--class-ref-map','porting/generated/reflection/class_ref_map.json','--class-list','porting/native_class_set.txt','--engine-classes','/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Engine/Classes','--source-root',str(root),'--report-dir',str(out/'reports')]
with (out/'generation.log').open('w') as log:result=subprocess.run(args,cwd=r,stdout=log,stderr=subprocess.STDOUT)
changed=[];added=[];restored=0
for p in root.rglob('*'):
 if not p.is_file():continue
 key=str(p.relative_to(root));old=before.get(key)
 if old is None:added.append(key)
 elif sha(p)==old['sha256']:os.utime(p,ns=(old['atime'],old['mtime']));restored+=1
 else:changed.append(key)
removed=[key for key in before if not (root/key).exists()]
report=dict(command=args,returncode=result.returncode,changed=sorted(changed),added=sorted(added),removed=removed,unchanged_mtimes_restored=restored)
(out/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:len(v) if isinstance(v,list) else v for k,v in report.items() if k!='command'}))
raise SystemExit(result.returncode or bool(removed))
