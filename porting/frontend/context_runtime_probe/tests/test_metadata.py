import sys,unittest,tempfile,subprocess,shutil
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
import verify_metadata as m
HERE=Path(__file__).resolve().parent
ORIG=Path('/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin')
class Metadata(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.tmp=tempfile.TemporaryDirectory();cls.d=Path(cls.tmp.name);cls.obj=cls.d/'fixture.o';cls.exe=cls.d/'fixture';cls.cpp=cls.d/'fixture.gen.cpp';cls.cpp.write_text('DEFINE_FUNCTION(UBlueprintContextLibrary::execGetContext)\nZ_Construct_UFunction_UBlueprintContextLibrary_GetContext\n')
  subprocess.run(['g++','-O0','-g','-c',str(HERE/'metadata_fixture.cpp'),'-o',str(cls.obj)],check=True);subprocess.run(['g++','-no-pie','-Wl,--build-id','-g',str(cls.obj),'-o',str(cls.exe)],check=True)
 @classmethod
 def tearDownClass(cls):cls.tmp.cleanup()
 def audit(self,cpp=None,obj=None,exe=None,debug=None):return m.audit(ORIG,cpp or self.cpp,obj or self.obj,exe or self.exe,debug or self.exe)
 def test_actual_elf_fixture_and_mutations(self):
  self.assertTrue(self.audit()['passed'])
  badcpp=self.d/'bad.cpp';badcpp.write_text('wrong generated output')
  with self.assertRaises(ValueError):self.audit(cpp=badcpp)
  elf=m.ObjectELF(self.obj);sym=elf.sym('Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics','NewProp_ClassE');sec,off,_,_=sym
  badobj=self.d/'bad.o';b=bytearray(self.obj.read_bytes());b[elf.sections[sec][4]+off+16]^=1;badobj.write_bytes(b)
  with self.assertRaises(ValueError):self.audit(obj=badobj)
  syms=m.symbols(self.exe);sym=m.one_syms(syms,'Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics','10FuncParamsE');linked=m.LinkedELF(self.exe);va=sym[0];load=next(x for x in linked.loads if x[0]<=va<x[1]);pos=load[2]+va-load[0]+64
  badexe=self.d/'bad-exe';b=bytearray(self.exe.read_bytes());b[pos]^=1;badexe.write_bytes(b)
  with self.assertRaises(ValueError):self.audit(exe=badexe)
  registration=m.one_syms(syms,'StaticRegisterNativesUBlueprintContextLibrary','E5Funcs');va=registration[0];load=next(x for x in linked.loads if x[0]<=va<x[1]);pos=load[2]+va-load[0]+8
  badreg=self.d/'bad-registration';b=bytearray(self.exe.read_bytes());b[pos]^=1;badreg.write_bytes(b)
  with self.assertRaises(ValueError):self.audit(exe=badreg)
  array=m.one_syms(syms,'Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics','12PropPointersE');va=array[0];load=next(x for x in linked.loads if x[0]<=va<x[1]);pos=load[2]+va-load[0]
  badorder=self.d/'bad-order';b=bytearray(self.exe.read_bytes());b[pos:pos+8],b[pos+8:pos+16]=b[pos+8:pos+16],b[pos:pos+8];badorder.write_bytes(b)
  with self.assertRaises(ValueError):self.audit(exe=badorder)
  prop=m.one_syms(syms,'Z_Construct_UFunction_UBlueprintContextLibrary_GetContext_Statics','NewProp_ClassE');va=prop[0];load=next(x for x in linked.loads if x[0]<=va<x[1]);pos=load[2]+va-load[0]+40
  badmeta=self.d/'bad-meta';b=bytearray(self.exe.read_bytes());b[pos]^=1;badmeta.write_bytes(b)
  with self.assertRaises(ValueError):self.audit(exe=badmeta)
  with self.assertRaises(ValueError):self.audit(exe=self.exe,debug=badobj)
if __name__=='__main__':unittest.main()
