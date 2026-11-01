#!/usr/bin/env python3
"""Policy guards and a portable generated-header lifecycle smoke test (not a UE ABI test)."""
import copy
import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('confirmation_policy', HERE/'policy.py')
p = importlib.util.module_from_spec(spec); spec.loader.exec_module(p)

class Info:
 def __init__(self, fn, kind, name, cpp):
  self.fn=fn; self.kind=kind; self.cpp_name=cpp; self.package='/Script/FortniteGame'; self.rec=copy.deepcopy(p.EXPECTED[name])
class Gate:
 def __init__(self):
  self.items={p.ENUM:Info(p.ENUM,'enum','EFortDialogResult','EFortDialogResult'),p.HANDLE:Info(p.HANDLE,'struct','FortDialogExternalLatentActionHandle','FFortDialogExternalLatentActionHandle')}
 def info(self,fn):return self.items.get(fn)
 def reason(self,info):return None

def description():return Info(p.DESCRIPTION,'struct','FortDialogDescription_NUI','FFortDialogDescription_NUI')
class TestPolicy(unittest.TestCase):
 def test_closure_and_fail_closed(self):
  g=Gate(); d=description(); m=p.apply_struct(g,d,{'used':{}})
  self.assertEqual(m['used'],{p.ENUM:{'Enum'},p.HANDLE:{'Struct'}})
  for change in ('size','property','fn','package','enum','handle','missing'):
   with self.subTest(change=change):
    g=Gate(); d=description()
    if change=='size':d.rec['size']=248
    if change=='property':d.rec['properties'][0]['name']='Other'
    if change=='fn':d.fn='0x1'
    if change=='package':d.package='/Script/Other'
    if change=='enum':g.items[p.ENUM].rec['enumerators'][0]['value']=99
    if change=='handle':g.items[p.HANDLE].rec['properties'][0]['kind']='Float'
    if change=='missing':del g.items[p.HANDLE]
    with self.assertRaises(ValueError):p.apply_struct(g,d,{'used':{}})
 def test_unrelated_type_unchanged(self):
  d=description();d.cpp_name='FOther';m={'used':{}};self.assertIs(p.apply_struct(Gate(),d,m),m)
 def test_lifecycle_with_portable_delegate_stubs(self):
  # Includes the exact generated declaration fragment; mock delegates model independent
  # ownership and copying. Engine behavior and ABI require separate UE validation.
  code=r'''#include <cassert>
#include <functional>
#include <memory>
#include <type_traits>
struct Brush { int value=32; }; struct Text { int value=7; }; struct Name { int value=0; };
struct ActionArray { int size=0; }; struct Widget {}; struct Handler {}; struct Sound {};
template<class... A> struct Delegate { std::function<void(A...)> fn; void Unbind(){fn={};} bool IsBound()const{return bool(fn);} };
enum class EFortDialogResult { Confirmed }; using FName=Name;
struct FFortDialogExternalLatentActionHandle { int value; };
using FNativeConfirmationResult=Delegate<EFortDialogResult,FName>;
using FNativeAdvancedConfirmationResult=Delegate<EFortDialogResult,FName,bool,FFortDialogExternalLatentActionHandle>;
struct FFortDialogDescription_NUI { Brush Icon; Text MessageHeader,MessageBody; ActionArray ConfirmButtonInputActions; FName DeclineButtonInputAction; Widget* AdditionalContent; Widget* LeftAdditionalContent; float DisplayTime; bool Dismissable,Cancelable,bShouldWaitForLatentActionOnConfirmAction; Handler* NotificationHandler; Sound* ShowSound;
''' + (HERE/'declarations.inc').read_text() + r'''
};
static_assert(!std::is_trivially_copy_assignable<FFortDialogDescription_NUI>::value, "copy must invoke members");
int main(){ int calls=0; auto token=std::make_shared<int>(1); std::weak_ptr<int> weak=token;
 { FFortDialogDescription_NUI a; assert(a.Icon.value==32 && a.MessageHeader.value==7 && a.MessageBody.value==7);
 assert(!a.AdditionalContent && !a.LeftAdditionalContent && !a.NotificationHandler && !a.ShowSound && a.DisplayTime==0 && a.Dismissable && !a.Cancelable && !a.bShouldWaitForLatentActionOnConfirmAction && !a.NativeResultCallback.IsBound() && !a.NativeAdvancedResultCallback.IsBound());
 a.NativeResultCallback.fn=[token,&calls](EFortDialogResult,FName){++calls;};
 a.NativeAdvancedResultCallback.fn=[&calls](EFortDialogResult,FName,bool b,FFortDialogExternalLatentActionHandle h){calls+=b?h.value:0;};
 FFortDialogDescription_NUI b=a; a.NativeResultCallback.Unbind(); token.reset(); assert(!weak.expired());
 b.NativeResultCallback.fn(EFortDialogResult::Confirmed,{}); b.NativeAdvancedResultCallback.fn(EFortDialogResult::Confirmed,{},true,{3}); assert(calls==4);
 FFortDialogDescription_NUI c; c=b; b.NativeResultCallback.Unbind(); assert(!weak.expired()); c.NativeResultCallback.Unbind(); assert(weak.expired());
 } assert(weak.expired()); }
'''
  with tempfile.TemporaryDirectory() as td:
   src=Path(td)/'test.cpp';exe=Path(td)/'test';src.write_text(code)
   subprocess.run(['c++','-std=c++14','-Wall','-Wextra','-Werror',str(src),'-o',str(exe)],check=True)
   subprocess.run([str(exe)],check=True)
if __name__=='__main__':unittest.main()
