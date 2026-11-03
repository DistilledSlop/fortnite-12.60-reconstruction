from pathlib import Path
import json,subprocess,time,shutil,hashlib
r=Path('/opt/fortnite/FortniteClient-12.60-CL-13477524');w=r/'porting/generated/frontend-strict-batch-15';stage=r/'porting/generated/frontend-probes/paper2d-matched-stage-3/LinuxNoEditor';binary=stage/'FortniteLinux/Binaries/Linux/FortniteLinux'
row=json.loads((w/'results.json').read_text())[0];assert row['exit']==0 and row['source_tree_unchanged']
shutil.copy2(row['binary']['path'],binary)
game_config=r/'porting/native/Config/DefaultGame.ini'; staged_game=stage/'FortniteLinux/Config/DefaultGame.ini'; shutil.copy2(game_config,staged_game)
input_config=r/'porting/native/Config/DefaultInput.ini'; staged_input=stage/'FortniteLinux/Config/DefaultInput.ini'; shutil.copy2(input_config,staged_input)
(w/'input-config-stage.json').write_text(json.dumps({'source':str(input_config),'stage':str(staged_input),'sha256':hashlib.sha256(input_config.read_bytes()).hexdigest()},indent=2)+'\n')
(w/'game-config-stage.json').write_text(json.dumps({'source':str(game_config),'stage':str(staged_game),'sha256':hashlib.sha256(game_config.read_bytes()).hexdigest()},indent=2)+'\n')
(w/'stage.json').write_text(json.dumps({'source':row['binary']['path'],'stage':str(binary),'expected_sha256':row['binary']['sha256'],'scope':'client ELF only; runner checks exact staged ELF hash'},indent=2)+'\n')
env=['env','-u','LD_LIBRARY_PATH','VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.json','SDL_AUDIODRIVER=dummy','LP_NUM_THREADS=8','xvfb-run','-a','python3'];base=['--build-results',str(w/'results.json'),'--binary',str(binary),'--cwd',str(stage),'--timeout','240']
cases=[]
cases.append(('root-attach',env+[str(r/'porting/frontend/root_widget_instance_probe/runner.py'),*base,'--out',str(w/'root-attach'),'--runtime-args',str(r/'porting/generated/frontend-native-evidence/button-base-click-runtime-3/root-runtime-args.json'),'--config',str(stage/'FortniteLinux/Config/DefaultEngine.ini'),'--attach']))
for name,cmd in cases:
 start=time.time_ns()
 with (w/(name+'.launcher.log')).open('xb') as log:result=subprocess.run(cmd,cwd=r,stdout=log,stderr=subprocess.STDOUT)
 (w/(name+'.launch.json')).write_text(json.dumps(dict(command=cmd,exit=result.returncode,start_ns=start,end_ns=time.time_ns()),indent=2)+'\n')
 p=w/name/'receipt.json';d=json.loads(p.read_text()) if p.exists() else {};t=d.get('transcript',{});v=d.get('viewport',{})
 print(json.dumps(dict(case=name,exit=result.returncode,passed=d.get('passed'),returncode=d.get('returncode'),rows=t.get('rows'),last_phase=t.get('last_phase'),viewport_phase=v.get('last_phase'),imports=t.get('dependency_error_count'))),flush=True)
 if result.returncode and name!='root-attach':raise SystemExit(result.returncode)
