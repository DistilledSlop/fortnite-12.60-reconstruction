"""Synthetic binding mechanics only; these fixtures establish no ELF/runtime pass."""
import contextlib,json,tempfile,unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch
import binder as b

class ResumeTests(unittest.TestCase):
 def fixture(self,root):
  prev=root/'previous';prev.mkdir();frozen=root/'frozen';frozen.mkdir()
  def write(path,value):path.write_text(json.dumps(value));return path
  st={'schema':'context-build-start-v1','workspace':str(root),'start_ns':1,'snapshot':{'source_tree':{},'engine':{'root':str(root)},'generator':'generator'}}
  write(prev/'start.json',st)
  ast={'context_start_sha256':b.digest(prev/'start.json'),'binder_sha256':b.OLD_BINDER_SHA,'contract_sha256':b.digest(b.CONTRACT),'metadata_tool_sha256':b.OLD_METADATA_SHA,'implementation_tool_sha256':b.digest(b.HERE/'implementation_link.py')}
  write(prev/'activation.start.json',ast)
  targets={}
  for target in ('FortniteLinux','FortniteLinuxServer'):
   log=write(prev/(target+'.log'),{});br=write(prev/(target+'.run.json'),{'schema':'context-build-run-v1','target':target,'exit':0,'start_sha256':b.digest(prev/'start.json'),'start_ns':2,'log_sha256':b.digest(log)})
   targets[target]={'log':b.binding(log),'build_run':b.binding(br)}
  write(prev/'build.json',{'schema':'context-build-v1','start_sha256':b.digest(prev/'start.json'),'finished_ns':3,'targets':targets})
  write(prev/'activation.build.json',{'schema':'activation-build-v1','context_build_sha256':b.digest(prev/'build.json'),'activation_start_sha256':b.digest(prev/'activation.start.json'),'finished_ns':4,'targets':{}})
  rows=[]
  for name in b.INHERITED:
   copy=frozen/name;copy.write_bytes((prev/name).read_bytes());rows.append(b.binding(copy))
  manifest=write(frozen/'runtime-evidence-manifest.json',rows)
  cp=write(root/'checkpoint.json',{'builds':[{'target':t,'exit':0} for t in targets],'runtime_evidence_manifest_sha256':b.digest(manifest),'build_finish_sha256':b.digest(prev/'build.json'),'activation_build_sha256':b.digest(prev/'activation.build.json')})
  release=write(frozen/'source-release.json',{'released':True,'native_source_build_stage_released':True,'all_phase_processes_terminal':True,'checkpoint_sha256':b.digest(cp),'checkpoint':str(cp),'released_ns':5})
  p=SimpleNamespace(tree=lambda x:{},engine_state=lambda x:st['snapshot']['engine'],write_new=lambda path,obj:write(path,obj))
  a=SimpleNamespace(out=root/'retry',previous=prev,parent_checkpoint=cp,release=release)
  return a,p
 def setup_patches(self,stack,a,p):
  stack.enter_context(patch.object(b,'PARENT_SHA',b.digest(a.parent_checkpoint)));stack.enter_context(patch.object(b,'RELEASE_SHA',b.digest(a.release)))
  stack.enter_context(patch.object(b,'source_check'));stack.enter_context(patch.object(b,'collect',return_value={}))
  stack.enter_context(patch.object(b,'context',return_value=(p,{})))
  original=b.digest
  stack.enter_context(patch.object(b,'digest',side_effect=lambda path:'generator' if Path(path).name=='gen_native_classes.py' else original(path)))
 def test_resume_preserves_build_epoch_and_refuses_build_finish(self):
  with tempfile.TemporaryDirectory() as d,contextlib.ExitStack() as stack:
   a,p=self.fixture(Path(d));self.setup_patches(stack,a,p);old=(a.previous/'start.json').read_bytes();b.resume(a)
   self.assertEqual((a.out/'start.json').read_bytes(),old);self.assertTrue((a.out/'start.json').is_symlink());self.assertEqual(b.load(a.out/'start.json')['start_ns'],1)
   for fn in (b.build,b.finish,b.resume):
    with self.assertRaises(ValueError):fn(a)
 def test_parent_receipt_source_tools_release_tamper_rejected(self):
  for target in ('checkpoint','release','start.json','build.json','activation.start.json','activation.build.json','tools','source'):
   for missing in (False,True):
    with self.subTest(target=target,missing=missing),tempfile.TemporaryDirectory() as d,contextlib.ExitStack() as stack:
     a,p=self.fixture(Path(d));self.setup_patches(stack,a,p);b.resume(a)
     path={'checkpoint':a.parent_checkpoint,'release':a.release}.get(target,a.previous/target)
     if target=='tools':
      stack.enter_context(patch.object(b,'tool_bindings',return_value={}))
     elif target=='source':
      stack.enter_context(patch.object(b,'source_check',side_effect=ValueError('changed/missing source')))
     elif missing:path.unlink()
     else:path.write_text('{}')
     with self.assertRaises((ValueError,KeyError,FileNotFoundError)):b.snapshot_check(p,{},a.out)
 def test_symlink_redirect_and_retry_epoch_rejected(self):
  for kind in ('redirect','epoch'):
   with tempfile.TemporaryDirectory() as d,contextlib.ExitStack() as stack:
    a,p=self.fixture(Path(d));self.setup_patches(stack,a,p);b.resume(a)
    if kind=='redirect':
     f=a.out/'start.json';data=f.read_bytes();f.unlink();f.write_bytes(data)
    else:
     f=a.out/'activation.retry-start.json';r=b.load(f);r['start_ns']=4;f.write_text(json.dumps(r))
    with self.assertRaises(ValueError):b.retry_check(p,{},a.out)
 def test_old_binder_cannot_be_blessed_by_current_tool_hash(self):
  with tempfile.TemporaryDirectory() as d,contextlib.ExitStack() as stack:
   a,p=self.fixture(Path(d));self.setup_patches(stack,a,p)
   stack.enter_context(patch.object(b,'OLD_BINDER_SHA','unreviewed'))
   with self.assertRaises(ValueError):b.resume(a)
 def test_each_runtime_is_new_and_baseline_exact(self):
  with tempfile.TemporaryDirectory() as d:
   out=Path(d);(out/'activation.retry-start.json').write_text('{"start_ns":10}')
   for case in ('activation','registry','lifecycle'):
    argv=b.activation_command(Path('/bin/fixture'),'activation','',[]) if case=='activation' else ['/bin/fixture','-ContextRuntimeProbe','-ContextProbeCase='+case,'-ContextProbeTravelMap=/Engine/Maps/Entry','-stdout']+b.BASELINE_ARGS
    (out/(case+'.run.json')).write_text(json.dumps({'start_ns':11,'end_ns':12,'argv':argv}))
   b.fresh_retry_runs(out)
   for case in ('activation','registry','lifecycle'):
    path=out/(case+'.run.json');good=path.read_text();r=json.loads(good)
    for key,value in (('start_ns',9),('end_ns',10),('argv',['/bin/fixture','-nullrhi'])):
     bad=dict(r);bad[key]=value;path.write_text(json.dumps(bad))
     with self.assertRaises(ValueError):b.fresh_retry_runs(out)
    path.write_text(good)
 def test_activation_argv_matches_known_context_startup_baseline(self):
  self.assertEqual(b.activation_command(Path('/binary'),'activation','',[]),['/binary','-ActivationBaseProbe','-stdout','FortniteLinux','/Engine/Maps/Entry','-vulkan','-windowed','-ResX=1280','-ResY=720','-nosound','-unattended','-log','-FortniteLegacySequencer','-FortniteLegacyFieldPaths','-NOCONSOLE'])
