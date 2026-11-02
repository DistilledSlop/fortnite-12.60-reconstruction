"""Focused C++ lifetime/reentry check; shim does not claim UE runtime parity."""
from pathlib import Path
import subprocess
import tempfile
import policy

HERE = Path(__file__).resolve().parent
SHIM = r'''
#pragma once
#include <algorithm>
#include <cstdint>
#include <map>
#include <memory>
#include <vector>
using int32=int32_t;
template<class T> struct TSharedPtr {
 std::shared_ptr<T> p; bool IsValid()const{return bool(p);}
 T& operator*()const{return *p;}
};
template<class T> struct TWeakPtr {
 std::weak_ptr<T> p;
 TSharedPtr<T> Pin()const{return {p.lock()};}
 bool IsValid()const{return !p.expired();}
 void Reset(){p.reset();}
 TWeakPtr& operator=(const TSharedPtr<T>& s){p=s.p;return *this;}
};
template<class T> struct TArray:std::vector<T> {
 using std::vector<T>::operator=;
 bool Contains(T x)const{return std::find(this->begin(),this->end(),x)!=this->end();}
 void Add(T x){this->push_back(x);}
 int32 Remove(T x){auto n=this->size();this->erase(std::remove(this->begin(),this->end(),x),this->end());return n-this->size();}
};
struct FCommonActionBindingRoute;
struct FCommonActionBindingRecord {int32 Id;TWeakPtr<FCommonActionBindingRoute> Route;};
namespace CommonActionBindingData {
 inline std::map<int32,FCommonActionBindingRecord> Records;
 inline FCommonActionBindingRecord* Find(int32 k){auto i=Records.find(k);return i==Records.end()?nullptr:&i->second;}
 inline int32 Remove(int32 k){return Records.erase(k);}
}
'''
MAIN = r'''
#include "CommonActionRouteBindingOperations.h"
#include <cassert>
#include <functional>
struct FCommonActionBindingRoute {TArray<int32> Handles;std::function<void()> Destroy;~FCommonActionBindingRoute(){if(Destroy)Destroy();}};
using namespace CommonActionRouteBindingOperations;
using namespace CommonActionBindingData;
int main(){
 auto a=std::make_shared<FCommonActionBindingRoute>();
 auto b=std::make_shared<FCommonActionBindingRoute>();
 TWeakPtr<FCommonActionBindingRoute> self; self.p=a;
 Records.emplace(3,FCommonActionBindingRecord{3,{}});
 int notified=0;
 Associate(a->Handles,self,*Find(3),[&]{++notified;assert(Find(3)->Route.Pin().p==a);assert(a.use_count()==1);});
 assert(notified==1 && a->Handles.Contains(3));
 // Duplicate association must neither replace a changed route nor notify.
 Find(3)->Route.p=b;
 Associate(a->Handles,self,*Find(3),[&]{assert(false);});
 assert(Find(3)->Route.Pin().p==b);
 // Removal is stable/all-match, leaves capacity and clears even another route.
 a->Handles={7,3,8,3,9}; auto capacity=a->Handles.capacity();
 Disassociate(a->Handles,3,[&]{++notified;assert(!Find(3)->Route.IsValid());});
 assert((a->Handles==std::vector<int32>{7,8,9})); assert(capacity==a->Handles.capacity());
 Disassociate(a->Handles,3,[&]{assert(false);});
 // Absent registry entry still notifies after a successful array removal.
 a->Handles.Add(42); Disassociate(a->Handles,42,[&]{++notified;}); assert(notified==3);
 // An expired weak self replaces the previous record reference with empty.
 TWeakPtr<FCommonActionBindingRoute> expired;
 Associate(a->Handles,expired,*Find(3),[&]{assert(!Find(3)->Route.IsValid());});
 // Registry mutation/reentrant association in notification is safe: the old
 // record and array may disappear. Associate must do no later access to them.
 Associate(b->Handles,self,*Find(3),[&]{Records.clear();}); assert(!Find(3));
 // Lifetime: unregister pins the actual route through removal callback, then
 // releases it before registry erase; destructor can change the live handle.
 Records.emplace(11,FCommonActionBindingRecord{11,{}});
 Records.emplace(12,FCommonActionBindingRecord{12,{}});
 Find(11)->Route.p=a;
 int32 handle=11; bool destroyed=false;
 a->Destroy=[&]{destroyed=true;assert(Find(11));assert(Find(12));handle=12;};
 Unregister(handle,[&](FCommonActionBindingRoute& r,int32 id){assert(id==11);a.reset();assert(!destroyed);Disassociate(r.Handles,3,[]{});});
 assert(destroyed && handle==-1 && Find(11) && !Find(12));
 // Sign is not a lookup gate, and absent entries still invalidate the handle.
 Records.emplace(-7,FCommonActionBindingRecord{-7,{}});handle=-7;
 Unregister(handle,[](auto&,int32){assert(false);});assert(handle==-1 && !Find(-7));
 handle=999;Unregister(handle,[](auto&,int32){assert(false);});assert(handle==-1);
}
'''

policy.validate_original_image('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
expected = policy.CONTRACT['windows'][0]['sha256']
policy.CONTRACT['windows'][0]['sha256'] = '0' * 64
try:
    try:
        policy.validate_original_image('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
    except ValueError:
        pass
    else:
        raise AssertionError('corrupt original window accepted')
finally:
    policy.CONTRACT['windows'][0]['sha256'] = expected
with tempfile.TemporaryDirectory(prefix='common-route-operations-') as tmp:
    target=Path(tmp)
    (target/'CommonActionBindingData.h').write_text(SHIM)
    (target/'CommonActionRouteBindingOperations.h').write_text((HERE/'CommonActionRouteBindingOperations.h').read_text())
    (target/'main.cpp').write_text(MAIN)
    subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer',str(target/'main.cpp'),'-o',str(target/'test')],check=True)
    subprocess.run([str(target/'test')],check=True)
print('PASS bounded original windows and C++ association/removal lifetime/reentry checks')
