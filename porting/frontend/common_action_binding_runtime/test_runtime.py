import copy
import importlib.util
from pathlib import Path
import subprocess
import tempfile
import unittest
from types import SimpleNamespace

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('binding_policy', HERE / 'policy.py')
policy = importlib.util.module_from_spec(spec)
spec.loader.exec_module(policy)

SHIM = r'''
#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <memory>
#include <cstdio>
using uint8=uint8_t; using uint32=uint32_t; using int32=int32_t;
#define COMMONUI_API
#define TEXT(x) x
#define DECLARE_DELEGATE_OneParam(Name, Type) using Name=TDelegate<void(Type)>
#define DECLARE_MULTICAST_DELEGATE_OneParam(Name, Type) using Name=TMulticastDelegate<void(Type)>
struct FMemory { static void Memcpy(void* a,const void* b,size_t n){std::memcpy(a,b,n);} };
struct FString : std::string { using std::string::string; FString(std::string s):std::string(s){} const char* operator*()const{return c_str();} template<class... T> static FString Printf(const char* f,T... args){char b[256]; std::snprintf(b,sizeof(b),f,args...); return FString(b);} };
struct FName { std::string s; FName(){} FName(const char* x):s(x){} bool IsNone()const{return s.empty();} FString ToString()const{return FString(s);} };
const FName NAME_None;
struct UObject { virtual ~UObject(){} FString GetName()const{return FString("Table");} };
struct UDataTable:UObject{};
struct FDataTableRowHandle { const UDataTable* DataTable=nullptr; FName RowName; };
struct FText { std::string s; bool IsEmpty()const{return s.empty();} };
struct FKey { int key=0; bool IsValid()const{return key!=0;} };
template<class T> using TWeakObjectPtr=T*;
template<class T> using TWeakPtr=std::weak_ptr<T>;
template<class T> struct TArray:std::vector<T>{ using std::vector<T>::operator=; T& AddDefaulted_GetRef(){this->emplace_back();return this->back();} };
template<class K,class V> struct TMap { std::map<K,V> m; V* Find(K k){auto i=m.find(k);return i==m.end()?nullptr:&i->second;} V& Add(K k,const V& v){return m.insert_or_assign(k,v).first->second;} int32 Remove(K k){return m.erase(k);} };
struct FSimpleDelegate { std::function<void()> f; };
template<class S> struct TDelegate { std::function<S> f; bool alive=true; bool IsBound()const{return bool(f) && alive;} };
template<class S> struct TMulticastDelegate { std::vector<TDelegate<S>> a; void Add(const TDelegate<S>& d){if(d.f) a.push_back(d);} };
struct FUIActionKeyMapping { FKey Key; float HoldTime; };
struct FUIInputAction { FText DefaultDisplayName; TArray<FUIActionKeyMapping> KeyMappings; };
struct FCommonInputTypeInfo { FKey Key; bool bActionRequiresHold=false; float HoldTime=0; };
struct FCommonInputActionData { FText DisplayName; FCommonInputTypeInfo KeyboardInputTypeInfo, TouchInputTypeInfo, GamepadInputTypeInfos[4]; };
'''
MAIN = r'''
#include "CommonActionBindingData.h"
#include <cassert>
#include <limits>
#if defined(__SSE__)
#include <xmmintrin.h>
#endif
struct Resolver:ICommonActionBindingDataResolver {
 FUIInputAction action; FCommonInputActionData row; int tag=0,legacy=0,index=0;
 const FUIInputAction& ResolveTag(FName)override{++tag;return action;}
 const FCommonInputActionData& ResolveRow(const FDataTableRowHandle&)override{++legacy;return row;}
 int32 ResolveGamepadIndex(UObject*)override{++index;return 1;}
};
int main(){
 Resolver r; FCommonActionBindingDescriptor d{}; d.Tag=FName("Tag");
 float nan; uint32 bits=0x7fc12345; FMemory::Memcpy(&nan,&bits,4);
 r.action.DefaultDisplayName.s="fallback";
 r.action.KeyMappings={{FKey{1},nan},{FKey{1},-0.0f},{FKey{0},1},{FKey{2},1}};
 d.Progress.f=[](float){}; d.Progress.alive=false; d.Flag20=5; d.Flag21=6; d.Flag22=7; d.InputMode=8; d.Event=9;
 auto a=CommonActionBindingData::BuildRecord(d,nullptr,r);
 assert(a.Id==0 && a.Name.s=="Tag" && a.Display.s=="fallback" && a.Hold.size()==2 && a.Press.size()==1);
 uint32 actual; FMemory::Memcpy(&actual,&a.Hold[0].Time,4); assert(actual==bits);
 assert(a.Flag20==5 && a.Flag21==6 && a.Flag22==7 && a.InputMode==8 && a.Event==9 && a.State==-1 && a.ProgressListeners.a.size()==1);
 assert(r.tag==1 && r.legacy==0 && r.index==0);
 d.Tag=FName(); d.DisplayOverride.s="override"; UDataTable table; d.Row.DataTable=&table; d.Row.RowName=FName("Row");
 r.row.KeyboardInputTypeInfo={FKey{1},true,nan}; r.row.TouchInputTypeInfo={FKey{2},false,2}; r.row.GamepadInputTypeInfos[1]={FKey{3},true,3};
 auto b=CommonActionBindingData::BuildRecord(d,nullptr,r);
 assert(b.Id==1 && b.Name.s=="Legacy_Table_Row" && b.Display.s=="override");
 assert(b.Press.size()==2 && b.Hold.size()==1 && b.Press[0].Key.key==1 && b.Press[1].Key.key==2 && b.Hold[0].Key.key==3);
 FMemory::Memcpy(&actual,&b.Press[0].Time,4); assert(actual==bits);
 assert(r.legacy==1 && r.index==1);
 uint32 counter=0xffffffffu; int32 wrapped=CommonActionBindingData::AllocateIdBits(counter);
 uint32 returned; FMemory::Memcpy(&returned,&wrapped,4); assert(returned==0xffffffffu && counter==0);
 assert(CommonActionBindingData::AllocateIdBits(counter)==0 && counter==1);
#if defined(__SSE__)
 // COMISS honors DAZ: positive subnormal compares positive with DAZ clear,
 // and as zero with DAZ set. Construct it by bits so fast-math cannot fold it.
 const unsigned saved=_mm_getcsr();
 uint32 subbits=1; float sub; FMemory::Memcpy(&sub,&subbits,4);
 d.Tag=FName("Tag"); r.action.KeyMappings.clear(); r.action.KeyMappings.push_back({FKey{4},sub});
 _mm_setcsr(saved & ~(1u<<6));
 auto normal=CommonActionBindingData::BuildRecord(d,nullptr,r);
 _mm_setcsr(saved | (1u<<6));
 auto daz=CommonActionBindingData::BuildRecord(d,nullptr,r);
 _mm_setcsr(saved);
 assert(normal.Hold.size()==1 && normal.Press.empty());
 assert(daz.Press.size()==1 && daz.Hold.empty());
 FMemory::Memcpy(&actual,&daz.Press[0].Time,4); assert(actual==subbits);
 d.Tag=FName(); r.row.KeyboardInputTypeInfo={FKey{4},true,sub};
 r.row.TouchInputTypeInfo.Key=FKey{0}; r.row.GamepadInputTypeInfos[1].Key=FKey{0};
 _mm_setcsr(saved & ~(1u<<6));
 auto legacyNormal=CommonActionBindingData::BuildRecord(d,nullptr,r);
 _mm_setcsr(saved | (1u<<6));
 auto legacyDaz=CommonActionBindingData::BuildRecord(d,nullptr,r);
 _mm_setcsr(saved);
 assert(legacyNormal.Hold.size()==1 && legacyNormal.Press.empty());
 assert(legacyDaz.Press.size()==1 && legacyDaz.Hold.empty());
 FMemory::Memcpy(&actual,&legacyDaz.Press[0].Time,4); assert(actual==subbits);
#endif
 CommonActionBindingData::Add(-1,a); assert(CommonActionBindingData::Find(-1)->Id==0);
 CommonActionBindingData::Add(-1,b); assert(CommonActionBindingData::Find(-1)->Id==1);
 assert(CommonActionBindingData::Remove(-1)==1 && CommonActionBindingData::Remove(-1)==0 && !CommonActionBindingData::Find(-1));
}
'''

class RuntimeTests(unittest.TestCase):
    def test_exact_type_guards(self):
        infos = {fn: SimpleNamespace(fn=fn, kind='struct', cpp_name='F'+rec['name'], package='/Script/CommonUI', rec=copy.deepcopy(rec)) for fn,rec in policy.CONTRACT.items()}
        gate = SimpleNamespace(info=infos.get, reason=lambda _:None)
        policy.validate_types(gate)
        for fn in infos:
            original=infos[fn].rec
            infos[fn].rec={}
            with self.assertRaises(ValueError): policy.validate_types(gate)
            infos[fn].rec=original
        gate.reason=lambda _:'unavailable'
        with self.assertRaises(ValueError): policy.validate_types(gate)

    def test_shim_fast_math(self):
        with tempfile.TemporaryDirectory() as td:
            p=Path(td)
            (p/'Engine').mkdir()
            (p/'CoreMinimal.h').write_text(SHIM)
            for name in ['Delegates/Delegate.h','Engine/DataTable.h','InputCoreTypes.h','FUIInputAction.h','FCommonInputActionData.h']:
                (p/name).parent.mkdir(parents=True, exist_ok=True)
                (p/name).write_text('#pragma once\n#include "CoreMinimal.h"\n')
            for name in ['CommonActionBindingData.h','CommonActionBindingData.cpp']:
                (p/name).write_text((HERE/name).read_text())
            (p/'main.cpp').write_text(MAIN)
            subprocess.run(['c++','-std=c++17','-O2','-ffast-math','-Wall','-Wextra','-Werror','-I',td,str(p/'CommonActionBindingData.cpp'),str(p/'main.cpp'),'-o',str(p/'test')],check=True)
            subprocess.run([str(p/'test')],check=True)

if __name__ == '__main__': unittest.main()
