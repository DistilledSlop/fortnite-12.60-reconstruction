#!/usr/bin/env python3
"""Record source inputs before a coordinated build and bind live output to its ELF."""
import argparse, hashlib, json, os, subprocess, time
from pathlib import Path
import unity
HERE=Path(__file__).resolve().parent
RECEIPT=Path('/tmp/native-context-runtime-candidate-1/frozen-final-1/receipt.json')
PATCH=Path('/tmp/native-context-runtime-candidate-1/candidate-integration.patch')
RH='d58a3485a207d96f8957214321faeda92c4b8e3216159c6c10e6a4ca93e87a77'
PH='b3d4db68521cd6946ccb96076ba2078be6c13669584ae9daa62b7b88e55d5d04'
ROOT_AUDIT=Path('/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/frontend-native-evidence/native-context-runtime-root-audit-1.json')
AH='d52405265c36dab4812a583edf171792bf5b4b12e5fb5919223a64adf65eb147'
def digest(p):
 h=hashlib.sha256()
 with open(p,'rb') as f:
  for b in iter(lambda:f.read(1048576),b''):h.update(b)
 return h.hexdigest()
def need(x,s):
 if not x:raise ValueError(s)
def write_new(p,obj):
 with open(p,'x') as f:json.dump(obj,f,sort_keys=True,indent=2);f.write('\n')
def tree(root):
 root=root.resolve();return {str(p.relative_to(root)):digest(p) for p in sorted(root.rglob('*')) if p.is_file() and not p.is_symlink()}
def engine_state(root):
 root=root.resolve()
 def git(*args): return subprocess.check_output(['git','-C',str(root),*args])
 head=git('rev-parse','HEAD').decode().strip()
 patch=git('diff','--binary','HEAD','--')
 status=git('status','--porcelain=v1','--untracked-files=no')
 names=git('ls-files','--others','--exclude-standard','-z','--','Engine/Source').split(b'\0')
 extra={os.fsdecode(n):digest(root/os.fsdecode(n)) for n in names if n and (root/os.fsdecode(n)).is_file()}
 return {'root':str(root),'head':head,'tracked_diff_sha256':hashlib.sha256(patch).hexdigest(),'tracked_status_sha256':hashlib.sha256(status).hexdigest(),'untracked_engine_source':extra}
def expected_source_hash(name):
 expected=json.loads((HERE/'source-expectations.json').read_text())
 found=[value for relative,value in expected.items() if Path(relative).name==name]
 need(len(found)==1,'unique audited source expectation '+name)
 return found[0]
def pin():
 need(digest(RECEIPT)==RH and digest(PATCH)==PH and digest(ROOT_AUDIT)==AH,'frozen candidate and root audit hashes')
 audit=json.loads(ROOT_AUDIT.read_text());need(audit.get('passed') is True and audit['candidate_receipt']['sha256']==RH and audit['candidate_patch']['sha256']==PH,'root semantic audit chain')
def start(a):
 pin();root=a.workspace.resolve();out=a.out.resolve();out.mkdir(parents=True,exist_ok=False)
 required={'porting/gen_native_classes.py':'gen_native_classes.py','porting/native/Source/BlueprintContext/Public/BlueprintContextRuntime.h':'BlueprintContextRuntime.h','porting/native/Source/BlueprintContext/Private/BlueprintContextRuntime.cpp':'BlueprintContextRuntime.cpp','porting/native/Source/BlueprintContext/Public/BlueprintContextCore.h':'BlueprintContextCore.h','porting/native/Source/FortniteGame/ContextRuntimeProbe.cpp':'ContextRuntimeProbe.cpp','porting/native/Source/FortniteGame/FortniteGame.cpp':'FortniteGame.cpp'}
 for rel,src in required.items():need(digest(root/rel)==expected_source_hash(src),'applied source mismatch '+rel)
 generated=root/'porting/native/Source/BlueprintContext/Generated';frozen=Path('/tmp/native-context-runtime-candidate-1/audited-final/Source/BlueprintContext/Generated')
 for name in ('UBlueprintContextLibrary.cpp','UBlueprintContextLibrary.h','UBlueprintContextBase.h'):need(digest(generated/name)==digest(frozen/name),'generated candidate mismatch '+name)
 base=(frozen/'UBlueprintContextBase.cpp').read_text().replace('#include \"BlueprintContextCore.h\"','#include \"BlueprintContextCore.h\"')
 base=base.replace('#include "BlueprintContextCore.h"','#include "BlueprintContextCore.h"\n#include "BlueprintContextRuntime.h"')
 base=base.replace('[this]() { return HasAnyFlags(RF_ClassDefaultObject); }, [this]() { ContextInitialize(); });','[this]() { return HasAnyFlags(RF_ClassDefaultObject); }, [this]() { BlueprintContextRuntime::ObserveContextInitialize(this, true); ContextInitialize(); BlueprintContextRuntime::ObserveContextInitialize(this, false); });',1)
 base=base.replace('[this]() { return HasAnyFlags(RF_ClassDefaultObject); }, [this]() { Deinitialize(); });','[this]() { return HasAnyFlags(RF_ClassDefaultObject); }, [this]() { BlueprintContextRuntime::ObserveContextDeinitialize(this, true); Deinitialize(); BlueprintContextRuntime::ObserveContextDeinitialize(this, false); });',1)
 need((generated/'UBlueprintContextBase.cpp').read_text()==base,'generated base observer mismatch')
 roots=[root/'porting/native/Source',root/'porting/gen_native_classes.py']
 snapshot={'source_tree':tree(roots[0]),'engine':engine_state(a.engine),'generator':digest(roots[1]),'instrumentation':{x:expected_source_hash(x) for x in required.values()}}
 write_new(out/'start.json',{'schema':'context-build-start-v1','workspace':str(root),'start_ns':time.time_ns(),'snapshot':snapshot,'candidate_receipt_sha256':RH,'candidate_patch_sha256':PH,'root_audit_sha256':AH})
 print(out/'start.json')
def build_target(a):
 pin();out=a.out.resolve();st=json.loads((out/'start.json').read_text());root=Path(st['workspace'])
 need(tree(root/'porting/native/Source')==st['snapshot']['source_tree'] and engine_state(Path(st['snapshot']['engine']['root']))==st['snapshot']['engine'] and digest(root/'porting/gen_native_classes.py')==st['snapshot']['generator'],'source changed before build')
 cmd=list(a.args);cmd=cmd[1:] if cmd and cmd[0]=='--' else cmd
 need(cmd and a.target in cmd,'build command must name exact target')
 log=out/(a.target+'.build.log');result=out/(a.target+'.build.run.json');need(not result.exists(),'new build receipt')
 started=time.time_ns()
 with log.open('xb') as f: status=subprocess.run(cmd,cwd=root,stdout=f,stderr=subprocess.STDOUT).returncode
 write_new(result,{'schema':'context-build-run-v1','target':a.target,'argv':cmd,'start_ns':started,'end_ns':time.time_ns(),'exit':status,'log_sha256':digest(log),'start_sha256':digest(out/'start.json')})
 print(result)
 need(status==0,'build failed; failure retained')
def finish(a):
 pin();out=a.out.resolve();st=json.loads((out/'start.json').read_text());root=Path(st['workspace']);need(tree(root/'porting/native/Source')==st['snapshot']['source_tree'] and engine_state(Path(st['snapshot']['engine']['root']))==st['snapshot']['engine'] and digest(root/'porting/gen_native_classes.py')==st['snapshot']['generator'],'source changed during build')
 records={}
 for target in ('FortniteLinux','FortniteLinuxServer'):
  base=root/'porting/native/Intermediate/Build/Linux/B4D820EA'/target
  cpp=base/'Inc/BlueprintContext/UBlueprintContextLibrary.gen.cpp'
  unit=unity.resolve(base/'Development/BlueprintContext',cpp,st['start_ns'])
  d={'cpp':cpp,**unit,'binary':root/'porting/native/Binaries/Linux'/target,'debug':root/'porting/native/Binaries/Linux'/(target+'.debug'),'log':out/(target+'.build.log')}
  records[target]={}
  for kind,p in d.items():
   need(p.is_file() and (kind in ('rsp','unity_source') or p.stat().st_mtime_ns>=st['start_ns']),'not a fresh '+target+' '+kind);records[target][kind]={'path':str(p.resolve()),'sha256':digest(p),'mtime_ns':p.stat().st_mtime_ns}
  log=d['log'].read_text(errors='replace');need('Total execution time:' in log and 'ERROR:' not in log and d['unity_source'].name in log,'build log compilation evidence '+target)
  runfile=out/(target+'.build.run.json');run=json.loads(runfile.read_text());need(run['schema']=='context-build-run-v1' and run['target']==target and run['exit']==0 and run['start_sha256']==digest(out/'start.json') and run['log_sha256']==digest(d['log']) and run['start_ns']>=st['start_ns'],'build invocation '+target)
  records[target]['build_run']={'path':str(runfile),'sha256':digest(runfile)}
 write_new(out/'build.json',{'schema':'context-build-v1','start_sha256':digest(out/'start.json'),'finished_ns':time.time_ns(),'targets':records})
 print(out/'build.json')
def build_live_command(binary,case,travel,args):
 extra=list(args);extra=extra[1:] if extra and extra[0]=='--' else extra
 need(case in ('registry','lifecycle') and travel.startswith('/'),'live case/travel map')
 return [str(binary),'-ContextRuntimeProbe','-ContextProbeCase='+case,'-ContextProbeTravelMap='+travel,'-stdout']+extra

def stop_process(proc,force=False):
 try:
  if force:proc.kill()
  else:proc.terminate()
 except ProcessLookupError:pass

def run(a):
 pin();out=a.out.resolve();build=json.loads((out/'build.json').read_text());need(digest(out/'start.json')==build['start_sha256'],'build start')
 rec=build['targets'][a.target]['binary'];binary=a.binary.resolve();need(binary.is_file() and digest(binary)==rec['sha256'],'staged binary must match built target hash')
 need(a.cwd.is_dir() and 180<a.timeout<=3600,'valid launch cwd and timeout')
 log=out/(a.case+'.runtime.log');result=out/(a.case+'.run.json');need(not result.exists(),'new case receipt');start=time.time_ns()
 cmd=build_live_command(binary,a.case,a.travel_map,a.args);timed_out=False;inode_ok=False;err=None
 with log.open('xb') as f:
  proc=subprocess.Popen(cmd,cwd=a.cwd.resolve(),stdout=f,stderr=subprocess.STDOUT,stdin=None,close_fds=True)
  try:
   exe=Path('/proc')/str(proc.pid)/'exe';st=os.stat(exe);bt=os.stat(binary);inode_ok=(st.st_dev,st.st_ino)==(bt.st_dev,bt.st_ino)
   if not inode_ok:err='actual process executable inode mismatch';stop_process(proc)
  except OSError as exc:
   err='could not inspect executable: '+str(exc);stop_process(proc)
  try:status=proc.wait(timeout=a.timeout)
  except subprocess.TimeoutExpired:
   timed_out=True;stop_process(proc)
   try:status=proc.wait(timeout=10)
   except subprocess.TimeoutExpired:stop_process(proc,True);status=proc.wait()
 binary_ok=digest(binary)==rec['sha256']
 write_new(result,{'schema':'context-run-v2','target':a.target,'case':a.case,'build_sha256':digest(out/'build.json'),'argv':cmd,'cwd':str(a.cwd.resolve()),'environment':{k:os.environ.get(k) for k in ('DISPLAY','VK_ICD_FILENAMES','SDL_AUDIODRIVER','LP_NUM_THREADS','LD_LIBRARY_PATH')},'pid':proc.pid,'start_ns':start,'end_ns':time.time_ns(),'exit':status,'timed_out':timed_out,'error':err,'log':{'path':str(log),'sha256':digest(log)},'binary_sha256':rec['sha256'],'proc_exe_inode_verified':inode_ok,'binary_after_verified':binary_ok})
 print(result)
 need(status==0 and inode_ok and binary_ok and not timed_out and err is None,'live run failed; failure retained')
def main():
 p=argparse.ArgumentParser();sp=p.add_subparsers(dest='action',required=True)
 q=sp.add_parser('start');q.add_argument('--workspace',type=Path,required=True);q.add_argument('--engine',type=Path,required=True);q.add_argument('--out',type=Path,required=True);q.set_defaults(fn=start)
 q=sp.add_parser('build');q.add_argument('--out',type=Path,required=True);q.add_argument('--target',choices=('FortniteLinux','FortniteLinuxServer'),required=True);q.add_argument('args',nargs=argparse.REMAINDER);q.set_defaults(fn=build_target)
 q=sp.add_parser('finish');q.add_argument('--out',type=Path,required=True);q.set_defaults(fn=finish)
 q=sp.add_parser('run');q.add_argument('--out',type=Path,required=True);q.add_argument('--target',choices=('FortniteLinux','FortniteLinuxServer'),required=True);q.add_argument('--case',choices=('registry','lifecycle'),required=True);q.add_argument('--binary',type=Path,required=True);q.add_argument('--cwd',type=Path,required=True);q.add_argument('--travel-map',default='/Engine/Maps/Entry');q.add_argument('--timeout',type=int,default=240);q.add_argument('args',nargs=argparse.REMAINDER);q.set_defaults(fn=run)
 a=p.parse_args();a.fn(a)
if __name__=='__main__':main()
