"""Synthetic ELF positives exercise verifier mechanics; never live UE evidence."""
import copy,importlib,os,subprocess,sys,tempfile,unittest
from pathlib import Path
sys.path.insert(0,'/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/frontend/context_runtime_probe')
import implementation_link as v
class ELFTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.temp=tempfile.TemporaryDirectory(prefix='activation-link-elf-');cls.d=Path(cls.temp.name);d=cls.d
  header='''struct FObjectInitializer {}; class UCommonActivatableWidget { bool active; public: UCommonActivatableWidget(const FObjectInitializer&); void ActivateWidget(); void DeactivateWidget(); bool IsActivated() const; virtual void NativeActivateRequest(); virtual void NativeDeactivateRequest(); virtual void NativeHandleActivated(); virtual void NativeHandleDeactivated(); static void execActivateWidget(UCommonActivatableWidget*); static void execDeactivateWidget(UCommonActivatableWidget*); static bool execIsActivated(UCommonActivatableWidget*); };'''
  (d/'base.h').write_text(header)
  (d/'base.cpp').write_text('''#include "base.h"
UCommonActivatableWidget::UCommonActivatableWidget(const FObjectInitializer&) : active(false) {}
void UCommonActivatableWidget::ActivateWidget() {if(!active)NativeActivateRequest();}
void UCommonActivatableWidget::DeactivateWidget() {if(active)NativeDeactivateRequest();}
bool UCommonActivatableWidget::IsActivated() const {return active;}
void UCommonActivatableWidget::NativeActivateRequest(){active=true;NativeHandleActivated();}
void UCommonActivatableWidget::NativeDeactivateRequest(){active=false;NativeHandleDeactivated();}
void UCommonActivatableWidget::NativeHandleActivated(){asm volatile(""::"r"(this),"r"("activation-synthetic"):"memory");}
void UCommonActivatableWidget::NativeHandleDeactivated(){asm volatile("nop"::"r"(this):"memory");}
''')
  (d/'glue.cpp').write_text('''#include "base.h"
void UCommonActivatableWidget::execActivateWidget(UCommonActivatableWidget* p){p->ActivateWidget();}
void UCommonActivatableWidget::execDeactivateWidget(UCommonActivatableWidget* p){p->DeactivateWidget();}
bool UCommonActivatableWidget::execIsActivated(UCommonActivatableWidget* p){return p->IsActivated();}
int main(){return 0;}
''')
  for name in ('base','glue'):subprocess.run(['/opt/fortnite/UnrealEngine/Engine/Extras/ThirdPartyNotUE/SDKs/HostLinux/Linux_x64/v16_clang-9.0.1-centos7/x86_64-unknown-linux-gnu/bin/clang++','-O2','-fno-pic','-c',str(d/(name+'.cpp')),'-o',str(d/(name+'.o'))],check=True,capture_output=True)
  subprocess.run(['g++','-no-pie','-Wl,--build-id',str(d/'base.o'),str(d/'glue.o'),'-o',str(d/'native')],check=True,capture_output=True)
 @classmethod
 def tearDownClass(cls):cls.temp.cleanup()
 def audit(self):return v.audit(self.d/'base.o',self.d/'glue.o',self.d/'native',self.d/'native')
 def test_synthetic_positive_all_methods_thunks(self):
  r=self.audit();self.assertTrue(r['passed']);self.assertEqual(len(r['implementation']),8);self.assertEqual(len(r['native_exec']),3);self.assertTrue(any('local_constant_sha256' in x for c in r['implementation'].values() for x in c['relocations']))
 def test_object_code_mutation(self):
  e=v.m.ObjectELF(self.d/'base.o');s=e.sym('UCommonActivatableWidget14ActivateWidget','Ev');original=(self.d/'base.o').read_bytes();bad=bytearray(original);bad[e.sections[s[0]][4]+s[1]]^=1
  try:
   (self.d/'base.o').write_bytes(bad)
   with self.assertRaisesRegex(ValueError,'exact relocated'):self.audit()
  finally:(self.d/'base.o').write_bytes(original)
 def test_linked_code_mutation(self):
  original=(self.d/'native').read_bytes();r=self.audit()['implementation']['ActivateWidget'];elf=v.m.LinkedELF(self.d/'native');segment=next(x for x in elf.loads if x[0]<=r['address']<x[1]);bad=bytearray(original);bad[segment[2]+r['address']-segment[0]]^=1
  try:
   (self.d/'native').write_bytes(bad)
   with self.assertRaisesRegex(ValueError,'exact relocated'):self.audit()
  finally:(self.d/'native').write_bytes(original)
 def test_relocation_destination_mutation(self):
  e=v.m.ObjectELF(self.d/'glue.o');s=e.sym('execActivateWidget','');syms=v.rows(self.d/'native',{s[3],'_ZN24UCommonActivatableWidget14ActivateWidgetEv'});syms['_ZN24UCommonActivatableWidget14ActivateWidgetEv']=(1,1)
  with self.assertRaises(ValueError):v.exact_code(e,s,v.m.LinkedELF(self.d/'native'),syms)
 def test_linked_literal_mutation(self):
  original=(self.d/'native').read_bytes();self.assertEqual(original.count(b'activation-synthetic'),1)
  try:
   (self.d/'native').write_bytes(original.replace(b'activation-synthetic',b'Xctivation-synthetic'))
   with self.assertRaisesRegex(ValueError,'local constant payload'):self.audit()
  finally:(self.d/'native').write_bytes(original)
 def test_native_thunk_missing_actual_call(self):
  from unittest.mock import patch
  original=v.relocs
  def wrong(obj,s):
   r=original(obj,s)
   return [] if 'execActivateWidget' in s[3] else r
  with patch.object(v,'relocs',side_effect=wrong):
   with self.assertRaisesRegex(ValueError,'actual native exec'):self.audit()
 def test_unsupported_relocation(self):
  from unittest.mock import patch
  e=v.m.ObjectELF(self.d/'glue.o');s=e.sym('execActivateWidget','');rels=v.relocs(e,s);syms=v.rows(self.d/'native',{s[3]})
  with patch.object(v,'relocs',return_value=[(rels[0][0],999,rels[0][2],rels[0][3])]):
   with self.assertRaisesRegex(ValueError,'unsupported'):v.exact_code(e,s,v.m.LinkedELF(self.d/'native'),syms)
 def test_recipe_inclusion_and_mutations(self):
  d=self.d;script=d/'link.sh';response=d/'target.response';driver=Path('/usr/bin/true');objects=[d/'base.o',d/'glue.o']
  script.write_text('"'+str(driver)+'" -fuse-ld=lld -Wl,--build-id -o "'+str(d/'native')+'" -Wl,@"'+str(response)+'"\n');original='\n'.join('"'+str(p)+'"' for p in objects);response.write_text(original)
  self.assertEqual(len(v.recipe(script,response,d/'native',objects)['objects']),2)
  for changed in [str(objects[0]),original+'\n'+str(objects[0]),original+'\n@nested',original+'\n-flto']:
   response.write_text(changed)
   with self.assertRaises(ValueError):v.recipe(script,response,d/'native',objects)
  response.write_text(original)
  with self.assertRaisesRegex(ValueError,'output'):v.recipe(script,response,d/'wrong',objects)
if __name__=='__main__':unittest.main()
