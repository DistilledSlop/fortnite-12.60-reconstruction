from pathlib import Path
import subprocess,time,json,hashlib
r=Path('/opt/fortnite/FortniteClient-12.60-CL-13477524')
w=r/'porting/generated/frontend-routing-batch-10';w.mkdir(exist_ok=False)
source=subprocess.check_output(['git','rev-parse','HEAD'],cwd=r,text=True).strip()
cmd=['/opt/fortnite/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh','FortniteLinux','Linux','Development','-project='+str(r/'porting/native/FortniteLinux.uproject'),'-NoHotReload']
start=time.time_ns()
with (w/'FortniteLinux.log').open('wb') as log:
 result=subprocess.run(cmd,cwd=r,stdout=log,stderr=subprocess.STDOUT)
binary=r/'porting/native/Binaries/Linux/FortniteLinux'
def sha(p):
 with p.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
row=dict(target='FortniteLinux',source_head=source,command=cmd,cwd=str(r),exit=result.returncode,start_ns=start,end_ns=time.time_ns(),log=str(w/'FortniteLinux.log'),log_sha256=sha(w/'FortniteLinux.log'),source_tree_unchanged=not subprocess.check_output(['git','status','--porcelain','--','porting/native/Source'],cwd=r,text=True).strip(),binary={'path':str(binary),'sha256':sha(binary),'size':binary.stat().st_size} if result.returncode==0 else None)
row['elapsed_seconds']=(row['end_ns']-start)/1e9
row['engine_source_head']=subprocess.check_output(['git','rev-parse','HEAD'],cwd='/opt/fortnite/UnrealEngine',text=True).strip()
(w/'results.json').write_text(json.dumps([row],indent=2)+'\n')
print(json.dumps({k:row[k] for k in ('target','exit','elapsed_seconds','source_tree_unchanged')}),flush=True)
raise SystemExit(result.returncode or (not row['source_tree_unchanged']))
