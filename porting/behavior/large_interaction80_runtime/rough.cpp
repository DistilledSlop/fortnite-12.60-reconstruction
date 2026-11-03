// Source-level draft of 0x14208b4f0. All call, object and stack ABIs remain open.
#include <cstdint>
#include <cmath>
using U = std::uint64_t; using I = std::int32_t; using B = std::uint8_t;
template<class T> T& at(U p, U off) { return *reinterpret_cast<T*>(p + off); }
template<U address, class... A> U call(A...);
template<U address, class... A> float call_float(A...);
template<class... A> U virt(U object, U slot, A...);
extern void memory_free(U);
extern I global_1478c504c;
// Source's stack layout is significant: ec is never initialized in this body;
// registration also receives adjacent locals whose exact hidden ABI is unknown.
struct RegistrationFrame {
  B stack108; B gap107[7]; std::uint32_t stack100; B gapfc[4];
  U callback; std::uint32_t callback_low_extra; std::uint32_t ec;
  U e8_related; U gap_e0; U clone; I clone_count; B gap_cc[4];
  alignas(8) B weak[8]; U c0; U b8; U gap_b0; U a8; U gap_a0;
  U inline_object[4];
};
// Reads the original unwritten ec stack word only when a callback is built.
extern std::uint32_t read_stack_ec(RegistrationFrame&);
extern U address_of_callback(U native_address);
extern U address_of_vtable(U native_address);
// FUN_1446278d0 takes additional stack state; preserve its distinct fourth-arg forms.
extern void register_bits(U manager, U handle, RegistrationFrame&, std::uint32_t bits);
extern void register_float(U manager, U handle, RegistrationFrame&, float delay);
extern void initialize_weak(void* destination, U object); // FUN_142ee3b30
extern U allocate_30(); // FUN_142bfd200(0, 0x30), third argument absent
extern U resize_to_zero(U pointer); // FUN_142bfd200(pointer, 0, 0)

static U manager(U owner) {
  U outer=at<U>(owner,0x188);
  return outer ? at<U>(outer,0xc0) : at<U>(owner,0x320);
}
static bool class_match(U cls, U desired) {
  I i=at<I>(desired,0x38);
  return i<=at<I>(cls,0x38) && at<U>(at<U>(cls,0x30), U(std::int64_t(i)*8))==desired+0x30;
}
static void clear_handle(U owner, U handle) {
  U m=manager(owner);
  if (call<0x144621fb0>(m,handle)) call<0x144627760>(m,at<U>(handle,0));
  at<U>(handle,0)=0;
}
// Construct the callback object and clone it into the registration frame. The
// ec word is carried through all registrations, matching the same stack slot.
static void set_callback(RegistrationFrame& f, U callback) {
  f.callback=address_of_callback(callback); f.callback_low_extra=0;
}
static U construct_callback(RegistrationFrame& f, U self, bool stateful, B state) {
  U obj=allocate_30();
  if (obj) {
    at<U>(obj,0)=address_of_vtable(stateful ? 0x145daca60 : 0x1459a9e80);
    initialize_weak(reinterpret_cast<void*>(obj+8),self);
    at<U>(obj,0x10)=f.callback;
    at<std::uint32_t>(obj,0x18)=f.callback_low_extra;
    at<std::uint32_t>(obj,0x1c)=read_stack_ec(f);
    if (stateful) at<B>(obj,0x20)=state;
    at<U>(obj,0x28)=call<0x142bad740>();
    at<U>(obj,0)=address_of_vtable(stateful ? 0x145dacab8 : 0x1459a9ed8);
  }
  return obj;
}
static void initialize_registration(RegistrationFrame& f, U obj) {
  f.clone=0; f.clone_count=0;
  if (obj) virt(obj,0x40,obj,&f.clone);
  initialize_weak(f.weak,0);
  f.stack100=0xbf800000; f.c0=0; f.b8=0; f.a8=0; f.stack108=0;
}
static void finish(RegistrationFrame& f, U obj) {
  if (f.b8) {
    U target=f.a8 ? f.a8 : reinterpret_cast<U>(f.inline_object);
    virt(target,0x10); // decompiler omitted arguments
  }
  if (f.clone_count != 0 && f.clone) {
    virt(f.clone,0x38,f.clone,0);
    if (f.clone) f.clone=resize_to_zero(f.clone);
    f.clone_count=0;
  }
  if (f.clone) memory_free(f.clone);
  if (obj) {
    virt(obj,0x38,obj,0);
    U remainder=resize_to_zero(obj);
    if (remainder) memory_free(remainder);
  }
}

void interaction_14208b4f0(U self, U callback, B mode) {
  if (at<B>(self,0xf0)!=3) return;
  U owner=virt(self,0x150); // no explicit receiver in decompilation
  B phase=at<B>(self,0x870);
  if ((B(phase-1)>4 && (phase!=8 || mode!=0)) || !owner) return;
  clear_handle(owner,self+0x6a0);
  U related=call<0x141641350>(owner);
  if (related && B(call<0x14199e430>(related))) {
    if (!callback) {
      if (at<U>(self,0xaa8)) callback=call<0x142414c60>();
    } else at<U>(self,0xaa8)=call<0x1416806f0>(callback);
    call<0x14178ee00>(callback,mode==0);
  }
  if (B(at<B>(self,0x870)-1)<5) call<0x142081260>(self+0x840,self+0x810);
  call<0x14208c5c0>(self);
  float duration=0.0f;
  if (at<I>(self,0x430)>1) duration=at<float>(self,0x7b4)+at<float>(self,0x7b0);
  if (mode==1 || (mode==2 && B(call<0x142099860>(self))))
    duration=duration+at<float>(self,0x7a8)+at<float>(self,0x7ac);
  if (std::fabs(duration)>1e-8f) duration+=at<float>(self,0x7b8);
  at<float>(self,0x7a4)=duration;
  bool positive=mode!=0 && !(duration<=0.0f);
  B special=0;
  if (mode==2 && !B(call<0x142099860>(self))) {
    special=1;
    at<float>(self,0x8d4)=at<float>(owner,0x540)-at<float>(self,0x8b0);
  }
  B state=positive ? 6 : B((special^1)+7);
  if (!mode) state=9;
  // Only these 10 bytes are assigned; remaining event stack storage is unknown.
  struct Event { U callback; B mode; B state; } event;
  event.callback=callback; event.mode=mode; event.state=state;
  call<0x1406a9dc0>(self+0x2d8,&event);
  call<0x1420a43d0>(self,state);
  call<0x1420a1b20>(self); call<0x1420a06b0>(self);
  if (at<U>(self,0x910)) {
    U type=call<0x1429034c0>();
    U object=call<0x143f74cc0>(at<U>(self,0x910),type);
    if (object) {
      type=call<0x1429034c0>();
      if (class_match(at<U>(object,0x10),type)) call<0x1406801c0>(object);
    }
  }
  float now=at<float>(owner,0x540);
  // Reuse the same original stack slots across all registrations.
  RegistrationFrame frame; frame.e8_related=related;
  U candidate;
  if (!at<U>(self,0x910) || !(candidate=call<0x141c7c520>()) ||
      !B(call<0x1418724a0>(candidate)) || positive) {
    set_callback(frame,0x14209f540);
    U m=manager(owner); std::uint32_t bits=at<std::uint32_t>(self,0x7a4);
    U obj=construct_callback(frame,self,false,0);
    initialize_registration(frame,obj);
    register_bits(m,self+0xb48,frame,bits); finish(frame,obj);
    set_callback(frame,0x1420a43d0);
    at<float>(self,0x8b8)=now+at<float>(self,0x7a4);
    obj=construct_callback(frame,self,true,B((special^1)+7));
    m=manager(owner); bits=std::uint32_t(at<U>(self,0x7b8));
    initialize_registration(frame,obj);
    register_bits(m,self+0xb50,frame,bits); finish(frame,obj);
  } else {
    clear_handle(owner,self+0xb48); clear_handle(owner,self+0xb50);
    at<float>(self,0x8b8)=now;
    call<0x14209f540>(self);
  }
  U other=call<0x141641300>(owner);
  if (special || !other || !related || !B(call<0x14199e380>(related))) return;
  call<0x1406801c0>(other); virt(other,0x828,other);
  if (!B(call<0x1419b0cc0>(related))) {
    at<std::uint32_t>(self,0x8b8)=at<std::uint32_t>(related,0x12e8); return;
  }
  float delay=call_float<0x1419920d0>(related);
  if (std::fabs(delay)<=1e-8f) {
    if (!global_1478c504c) call<0x14209d550>(self);
    else call<0x14209d340>(); // arguments missing from decompilation
  } else {
    set_callback(frame,0x14209d320);
    U m=manager(owner);
    U obj=construct_callback(frame,self,false,0);
    initialize_registration(frame,obj);
    register_float(m,self+0x280,frame,delay); finish(frame,obj);
  }
  at<float>(self,0x8b8)=delay+now;
}
