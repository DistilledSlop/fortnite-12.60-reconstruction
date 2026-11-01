import os,sys,tempfile,time,unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import unity,provenance
class Unity(unittest.TestCase):
 def test_preserved_recipe_dates_do_not_hide_a_stale_object(self):
  with tempfile.TemporaryDirectory() as t:
   root=Path(t);gen=root/'UBlueprintContextLibrary.gen.cpp';gen.write_text('// newly generated metadata')
   unit=root/'Module.BlueprintContext.gen.cpp';unit.write_text('#include "'+str(gen)+'"\n')
   obj=root/'Module.BlueprintContext.gen.cpp.o';obj.write_bytes(b'fresh object')
   rsp=root/(obj.name+'.rsp');rsp.write_text('-c '+str(unit)+' -o '+str(obj))
   start=min(gen.stat().st_mtime_ns,obj.stat().st_mtime_ns)-1
   for p in (unit,rsp):os.utime(p,ns=(start-1000,start-1000))
   self.assertEqual(unity.resolve(root,gen,start)['object'],obj)
   os.utime(obj,ns=(start-1,start-1))
   with self.assertRaises(ValueError):unity.resolve(root,gen,start)
 def test_fresh_unity_exact_source_and_mutations(self):
  with tempfile.TemporaryDirectory() as t:
   root=Path(t);dev=root/'Development';dev.mkdir();inc=root/'Inc';inc.mkdir();gen=inc/'UBlueprintContextLibrary.gen.cpp';gen.write_text('// fresh generated')
   unit=dev/'Module.BlueprintContext.gen.cpp';unit.write_text('#include "'+str(gen)+'"\n');obj=dev/'Module.BlueprintContext.gen.cpp.o';obj.write_bytes(b'object');rsp=dev/(obj.name+'.rsp');rsp.write_text('-c '+str(unit)+' -o '+str(obj));start=min(x.stat().st_mtime_ns for x in (gen,unit,obj,rsp))-1
   self.assertEqual(unity.resolve(dev,gen,start)['object'],obj)
   unit.write_text('#include "wrong.gen.cpp"\n')
   with self.assertRaises(ValueError):unity.resolve(dev,gen,start)
   unit.write_text('#include "'+str(gen)+'"\n#include "'+str(gen)+'"\n')
   with self.assertRaises(ValueError):unity.resolve(dev,gen,start)
   unit.write_text('#include "'+str(gen)+'"\n');rsp.write_text('-c '+str(unit)+' -o '+str(dev/'wrong.o'))
   with self.assertRaises(ValueError):unity.resolve(dev,gen,start)
   rsp.write_text('-c '+str(unit)+' -o '+str(obj));os.utime(obj,ns=(start-1,start-1))
   with self.assertRaises(ValueError):unity.resolve(dev,gen,start)
 def test_standalone_and_duplicate(self):
  with tempfile.TemporaryDirectory() as t:
   d=Path(t);gen=d/'UBlueprintContextLibrary.gen.cpp';gen.write_text('// generated');o=d/'UBlueprintContextLibrary.gen.cpp.o';o.write_bytes(b'x');r=d/(o.name+'.rsp');r.write_text('-c '+str(gen)+' -o '+str(o));start=min(x.stat().st_mtime_ns for x in (gen,o,r))-1;self.assertEqual(unity.resolve(d,gen,start)['object'],o)
   u=d/'Module.BlueprintContext.gen.cpp';u.write_text('#include "'+str(gen)+'"\n');o2=d/(u.name+'.o');o2.write_bytes(b'y');r2=d/(o2.name+'.rsp');r2.write_text('-c '+str(u)+' -o '+str(o2))
   with self.assertRaises(ValueError):unity.resolve(d,gen,start)
 def test_direct_staged_command_is_automatic(self):
  c=provenance.build_live_command(Path('/stage/FortniteLinux'),'lifecycle','/Engine/Maps/Entry',['--','-basedir=/stage','-foo'])
  self.assertEqual(c[0],'/stage/FortniteLinux');self.assertIn('-ContextProbeCase=lifecycle',c);self.assertIn('-ContextProbeTravelMap=/Engine/Maps/Entry',c);self.assertIn('-basedir=/stage',c);self.assertNotIn('--',c)
  with self.assertRaises(ValueError):provenance.build_live_command(Path('/stage/app'),'lifecycle','bad',[])
if __name__=='__main__':unittest.main()
