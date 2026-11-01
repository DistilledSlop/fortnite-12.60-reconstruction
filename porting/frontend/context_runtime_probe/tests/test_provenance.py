import sys,tempfile,unittest,subprocess,os,json
from unittest.mock import patch
from types import SimpleNamespace
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import provenance as p,verify_v1
class Provenance(unittest.TestCase):
 def test_engine_tracking_and_exclusive_receipt(self):
  with tempfile.TemporaryDirectory() as t:
   d=Path(t);subprocess.run(['git','init','-q',str(d)],check=True);subprocess.run(['git','-C',str(d),'config','user.name','Probe Test'],check=True);subprocess.run(['git','-C',str(d),'config','user.email','probe@example.invalid'],check=True)
   f=d/'Engine/Source/a.txt';f.parent.mkdir(parents=True);f.write_text('one');subprocess.run(['git','-C',str(d),'add','.'],check=True);subprocess.run(['git','-C',str(d),'commit','-qm','initial'],check=True)
   a=p.engine_state(d);f.write_text('two');self.assertNotEqual(a,p.engine_state(d));f.write_text('one');extra=f.parent/'new.txt';extra.write_text('untracked');self.assertNotEqual(a,p.engine_state(d))
   out=d/'new.json';p.write_new(out,{'a':1})
   with self.assertRaises(FileExistsError):p.write_new(out,{'a':2})
 def test_timeout_receipt_is_preserved(self):
  with tempfile.TemporaryDirectory() as tmp:
   d=Path(tmp);binary=Path(sys.executable).resolve();(d/'start.json').write_text('{}');(d/'build.json').write_text(json.dumps({'start_sha256':p.digest(d/'start.json'),'targets':{'FortniteLinux':{'binary':{'path':str(binary),'sha256':p.digest(binary)}}}}))
   class Fake:
    pid=os.getpid()
    def __init__(self):self.n=0
    def wait(self,timeout=None):
     self.n+=1
     if self.n==1:raise subprocess.TimeoutExpired('fixture',timeout)
     return -15
    def terminate(self):pass
   a=SimpleNamespace(out=d,target='FortniteLinux',binary=binary,cwd=d,timeout=181,case='registry',travel_map='/Engine/Maps/Entry',args=[])
   with patch.object(p,'pin'),patch.object(p.subprocess,'Popen',return_value=Fake()):
    with self.assertRaises(ValueError):p.run(a)
   receipt=json.loads((d/'registry.run.json').read_text());self.assertTrue(receipt['timed_out']);self.assertTrue(receipt['proc_exe_inode_verified']);self.assertEqual(receipt['exit'],-15)
 def test_log_mutations(self):
  good='\n'.join('CONTEXT_PROBE_V1 '+x+' PASS' for x in verify_v1.REQUIRED)+'\nCONTEXT_PROBE_V1 END\n';self.assertTrue(verify_v1.verify(good))
  for bad in (good.replace(' PASS',' FAIL',1),good.replace('CONTEXT_PROBE_V1 END',''),good+good):self.assertFalse(verify_v1.verify(bad))
if __name__=='__main__':unittest.main()
