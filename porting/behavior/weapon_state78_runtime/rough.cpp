// Connected source-level lift; adapters and object layout require ABI recovery.
#include <cstdint>
#include <initializer_list>
using U = std::uint64_t; using I = std::int32_t; using B = std::uint8_t;
template<class T> T& at(U p, U off) { return *reinterpret_cast<T*>(p + off); }
template<U Address, class... A> U call(A...); // address-specific ABI, including implicit native arguments
// Dispatch through the vtable pointer stored at subobject, without inventing a receiver.
template<class... A> U vtable_field_call(U subobject, U slot, A...);
template<class... A> U virt(U object, U slot, A...);
extern B weak_valid(U); extern U weak_get(U); extern void memory_free(U);
extern B stub_return_false(); // retain call and branch; implementation not assumed
extern I global_1478de4dc;
// Exact offset depends on the decompiler's UObject layout: *(UObject**)weak[14].OuterPrivate.
extern U weak14_outer_indirect(U weak_object);
// The source uses *(longlong*)object->ClassPrivate, unlike owner+0x10.
extern U indirect_class_private(U object);
struct Iterator { U array; I index; }; // only fields written/read by recovered source

static bool class_match(U object_class, U desired) {
  I index=at<I>(desired,0x38);
  return index<=at<I>(object_class,0x38) &&
      at<U>(at<U>(object_class,0x30), U(std::int64_t(index)*8))==desired+0x30;
}

U weapon_1426a07c0(U w) {
  U result=call<0x143f76650>();
  U owner=call<0x141125940>(w);
  if (result) return result;
  if (owner) {
    U cls=call<0x142a75f10>();
    if (class_match(at<U>(owner,0x10),cls)) {
      cls=call<0x142a75f10>();
      if (class_match(at<U>(owner,0x10),cls) && weak_valid(owner+0x26a0)) {
        U obj=weak_get(owner+0x26a0);
        result=0;
        U value=weak14_outer_indirect(obj);
        if (value) return value;
      }
    }
  }
  U obj=at<U>(w,0xe0);
  if (obj) {
    U cls=call<0x1446f1730>();
    if (class_match(indirect_class_private(obj),cls)) return obj;
    result=0;
  }
  if (virt(w,0x150,w)) {
    U source=virt(w,0x150,w);
    Iterator it; // helper initializes the observed fields; no blanket zero-initialization
    call<0x14468fb70>(source,&it);
    while (it.index>=0 && it.index<at<I>(it.array,8)) {
      obj=weak_get(at<U>(it.array,0)+U(std::int64_t(it.index)*8));
      if (obj) {
        U cls=call<0x142a69680>();
        I index=at<I>(cls,0x38); U object_class=indirect_class_private(obj);
        if (index<=at<I>(object_class,0x38) &&
            at<U>(at<U>(object_class,0x30),U(std::int64_t(index)*8))==cls+0x30 &&
            call<0x1423d4240>(obj,U(std::int64_t(I((std::uint32_t(index) & 0xffffff00U)|1U))))==owner)
          return obj;
      }
      ++it.index;
    }
  }
  return result;
}

void weapon_142694dd0(U w) {
  U owner=call<0x141125940>(); // decompiler omitted arguments
  if (!owner) return;
  U cls=call<0x142a174f0>();
  if (!class_match(at<U>(owner,0x10),cls)) return;
  U manager=vtable_field_call(owner+0x4c8,0x10); // no explicit receiver in source
  if (!manager || at<I>(w,0x928)==-1 || at<B>(w,0xf0)==3) return;
  U item=call<0x140708a10>(manager,at<I>(w,0x928));
  if (!item) return;
  U object=call<0x14070f400>(item);
  if (!object) return;
  cls=call<0x1428f53b0>();
  if (class_match(at<U>(object,0x10),cls)) {
    U type=call<0x142b1a050>();
    if (B(call<0x142d6a820>(at<U>(object,0x10),type)) && stub_return_false())
      call<0x142b216e0>(object);
  }
}

void weapon_1426aeed0(U w) {
  U owner=call<0x141125940>(); // decompiler omitted arguments
  if (!owner) return;
  U cls=call<0x142a174f0>();
  if (!class_match(at<U>(owner,0x10),cls)) return;
  U manager=vtable_field_call(owner+0x4c8,0x10); // source supplies no arguments
  if (!manager) return;
  weapon_142694dd0(w);
  if (at<U>(w,0x350) && at<B>(w,0xf0)==3) {
    for (U off : {U(0x928),U(0x92c),U(0x930),U(0x934)}) {
      if (at<I>(w,off)!=-1) { call<0x1406fedb0>(manager,w+off); at<I>(w,off)=-1; }
    }
    std::uint32_t i=0;
    if (at<I>(w,0x940)>0) do {
      if (at<I>(at<U>(w,0x938),U(i)*4)!=-1)
        call<0x1406fedb0>(manager,at<U>(w,0x938)+U(i)*4);
      ++i;
    } while (I(i)<at<I>(w,0x940));
    at<I>(w,0x940)=0;
    if (at<I>(w,0x944)!=0) call<0x14072d2e0>(w+0x938,0);
    i=0;
    if (at<I>(w,0x950)>0) do {
      U element=at<U>(w,0x948)+U(i)*0x28;
      if (B(call<0x1406a7ed0>(element))) call<0x141611250>(element);
      ++i;
    } while (I(i)<at<I>(w,0x950));
    I count=at<I>(w,0x950);
    if (count!=0) {
      U p=at<U>(w,0x948)+8;
      do {
        if (at<U>(p,0x10)) memory_free(at<U>(p,0x10));
        if (at<U>(p,0)) memory_free(at<U>(p,0));
        p+=0x28; --count;
      } while (count!=0);
    }
    at<I>(w,0x950)=0;
    if (at<I>(w,0x954)!=0) call<0x140690fd0>(w+0x948,0);
    at<std::uint32_t>(w,0x900)|=4;
  }
  if (global_1478de4dc!=0) {
    U type=call<0x14278c330>();
    U object=virt(w,0x610,w,type);
    if (object) call<0x141cce690>(object,manager);
  }
}
