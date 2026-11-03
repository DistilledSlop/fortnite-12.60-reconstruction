// Uncounted connected lifts. Addresses and native ABI/layout contracts remain unresolved.
#include <cstdint>
#include <cstring>
namespace pawn_interact76 {
using P=std::uintptr_t; using u8=std::uint8_t; using u32=std::uint32_t; using u64=std::uint64_t;
template<class T> T rd(P p, std::intptr_t o=0) { T v; std::memcpy(&v,(void*)(p+o),sizeof v); return v; }
template<class T> void wr(P p,std::intptr_t o,T v) { std::memcpy((void*)(p+o),&v,sizeof v); }
extern P world(P); // vtable +150
extern P lockon_type(); // 142741b60
extern void notify_interacting_pawn(P); // 143f72040
extern P alloc(std::size_t); extern P realloc_native(P,std::size_t); extern void free_native(P);
extern P delegate_base_vtable(),delegate_final_vtable(),timer_callback(); // data 1459a9e80/ed8, code 141be85a0
extern void weak_init(P storage,P object); // 142ee3b30
extern P delegate_identity(); // 142bad740
extern void delegate_copy_to(P original,P out_pointer_and_count); // original vtable +40
extern void delegate_destroy(P object); // vtable +38
extern void temporary_delegate_destroy(P object); // vtable +10
extern void set_timer(P manager,P handle,P delegate_pointer_and_count,float seconds,P scratch); // 1446278d0; hidden temporary ABI unresolved
extern P find_timer(P manager,P handle); extern void clear_timer(P manager,u64 handle); // 144621fb0, FTimerManager_InternalClearTimer
extern u8 log_level();
extern P enum_lookup(); // 142d68a40 then 142ed84a0 for Engine.ENetRole
extern void missing_enum_diagnostic_and_fatal(); // log at 1459a4bc8:92c then 142c7b670
extern P role_string(P enumeration,u8 role); // 142d67ac0; allocated string
extern P pawn_name_string(P pawn); // 142ce9f90 with native hidden arguments
extern void log_interaction(P pawn_name,P role_name,int variant); // level 5; 0 set, 1 clear, 2 wrong pawn
extern P timer_temporary_inline_storage(P scratch); // native stack-local object location
extern P timer_temporary_external(P scratch); extern bool timer_temporary_engaged(P scratch);
extern void timer_temporary_init(P scratch); // weak_init(0), zero remaining fields; ABI placement unresolved
static P timer_manager(P w) { P sub=rd<P>(w,0x188); return sub?rd<P>(sub,0xc0):rd<P>(w,800); }
static bool is_a(P obj,P type) { P cls=rd<P>(obj,0x10); int i=rd<int>(type,0x38); return rd<int>(cls,0x38)>=i && rd<P>(rd<P>(cls,0x30),i*8)==type+0x30; }
static void interaction_log(P pawn,int variant) {
  u8 role=rd<u8>(pawn,0xf0); P e=enum_lookup(); if(!e) missing_enum_diagnostic_and_fatal();
  P r=role_string(e,role); P n=pawn_name_string(pawn); log_interaction(n,r,variant);
  if(n) free_native(n); if(r) free_native(r);
}
// Actual decompiled machine arguments: (pawn, object, low-byte clear), despite
// generated prototype (pawn, bool, UObject*). Resolve reflected thunk ABI.
void ServerSetInteractingItem(P pawn,P object,u8 clear) {
  if(!object) return;
  P type=lockon_type(); if(!is_a(object,type)) return;
  if(!clear) {
    if(rd<P>(object,0x818)) return;
    wr<P>(object,0x818,pawn); notify_interacting_pawn(object);
    wr<P>(pawn,10440,object);
    if(!world(pawn)) return;
    P manager=timer_manager(world(pawn));
    P original=alloc(0x30);
    if(original) {
      wr<P>(original,0,delegate_base_vtable()); weak_init(original+8,object);
      wr<u64>(original,0x10,timer_callback()); wr<u32>(original,0x18,0);
      // +0x1c is copied from an uninitialized source stack word, not a default.
      extern u32 delegate_stack_word_1c(); wr<u32>(original,0x1c,delegate_stack_word_1c());
      wr<P>(original,0x28,delegate_identity()); wr<P>(original,0,delegate_final_vtable());
    }
    struct Copy { P ptr; int count; } copy{0,0};
    if(original) delegate_copy_to(original,(P)&copy);
    alignas(8) u8 scratch[64]; timer_temporary_init((P)scratch);
    set_timer(manager,object+0x810,(P)&copy,6.0f,(P)scratch);
    if(timer_temporary_engaged((P)scratch)) {
      P ext=timer_temporary_external((P)scratch);
      temporary_delegate_destroy(ext?ext:timer_temporary_inline_storage((P)scratch));
    }
    if(copy.count && copy.ptr) { delegate_destroy(copy.ptr); copy.ptr=realloc_native(copy.ptr,0); copy.count=0; }
    if(copy.ptr) free_native(copy.ptr);
    if(original) { delegate_destroy(original); P residual=realloc_native(original,0); if(residual) free_native(residual); }
    if(log_level()>=5) interaction_log(pawn,0);
  } else if(rd<P>(object,0x818)==pawn) {
    if(world(pawn)) {
      P manager=timer_manager(world(pawn));
      if(find_timer(manager,object+0x810)) clear_timer(manager,rd<u64>(object,0x810));
      wr<u64>(object,0x810,0);
      if(log_level()>=5) interaction_log(pawn,1);
    }
    wr<P>(object,0x818,0); notify_interacting_pawn(object);
  } else if(log_level()>=5) interaction_log(pawn,2);
}
extern bool is_server_world(P); // 144693a30
extern bool newer_timestamp(u32 incoming,u32 current); // 142331fa0
extern void acceleration_changed(P); // 142339240
extern bool movement_differs(P current,P incoming); // 141a0a860
extern P replicated_movement_destination(P); // 14070f0d0
extern void pawn_v420(P,P captured_vtable); extern void pawn_vad8(P); extern void pawn_vae0(P);
extern void pawn_v780(P,P captured_vtable); extern void balloon_changed(P,bool); // 14233c8a0
extern void pawn_v12d8(P,P captured_vtable,bool); extern void pawn_v12d0(P,P captured_vtable); extern void vehicle_changed(P); // 14233c9a0
extern void pawn_v1448(P,P);
void FastSharedReplication(P p,P s) {
  if(is_server_world(world(p)) || rd<u8>(p,0xf0)!=1) return;
  constexpr int quick=11256, balloon=4323, vehicle=4192;
  u32 timestamp=rd<u32>(s,0x34);
  if(!newer_timestamp(timestamp,rd<u32>(p,quick+0x94))) return;
  wr<u32>(p,0x310,timestamp); wr<u32>(p,quick+0x94,timestamp);
  if(rd<u8>(p,0x318)!=rd<u8>(s,0x47)) {
    wr<u8>(p,0x318,rd<u8>(s,0x47));
    P movement=rd<P>(p,0x280); wr<u8>(movement,0x38b,rd<u8>(movement,0x38b)|0x80);
    movement=rd<P>(p,0x280); wr<u8>(movement,0x38b,rd<u8>(movement,0x38b)|0x40);
  }
  wr<u32>(p,9840,rd<u32>(s,0x40));
  if(rd<std::uint16_t>(p,8220)!=rd<std::uint16_t>(s,0x44) || rd<u8>(p,8222)!=rd<u8>(s,0x46)) {
    wr<std::uint16_t>(p,8220,rd<std::uint16_t>(s,0x44)); wr<u8>(p,8222,rd<u8>(s,0x46)); acceleration_changed(p);
  }
  if(movement_differs(p+quick+0x60,s)) {
    P dst=replicated_movement_destination(p);
    // Preserve source read/write order and width, including potential aliasing.
    u64 a=rd<u64>(s,8); wr<u64>(dst,0,rd<u64>(s)); wr<u64>(dst,8,a);
    for(int base=0x10;base<=0x20;base+=0x10) {
      u32 b=rd<u32>(s,base+4),c=rd<u32>(s,base+8),d=rd<u32>(s,base+12);
      wr<u32>(dst,base,rd<u32>(s,base)); wr<u32>(dst,base+4,b); wr<u32>(dst,base+8,c); wr<u32>(dst,base+12,d);
    }
    wr<u32>(dst,0x30,rd<u32>(s,0x30)); P vt=rd<P>(p); wr<u8>(p,quick+0x98,1); pawn_v420(p,vt); wr<u8>(p,quick+0x98,0);
  }
  if(rd<u8>(p,1952)!=rd<u8>(s,0x48)) { wr<u8>(p,1952,rd<u8>(s,0x48)); pawn_vad8(p); }
  if(rd<u8>(p,1953)!=rd<u8>(s,0x49)) { wr<u8>(p,1953,rd<u8>(s,0x49)); pawn_vae0(p); }
  wr<u8>(p,1339,rd<u8>(s,0x4a));
  u32 bits=rd<u32>(p,0x320); bits=((u32(rd<u8>(s,0x4b))*2^bits)&2)^bits; wr<u32>(p,0x320,bits);
  if((bits&1)!=rd<u8>(s,0x4c)) { P vt=rd<P>(p); wr<u32>(p,0x320,((rd<u8>(s,0x4c)^bits)&1)^bits); pawn_v780(p,vt); }
  u8 v=rd<u8>(s,0x4d); if(rd<u8>(p,balloon+2)!=v) { wr<u8>(p,balloon+2,v); balloon_changed(p,v==0); }
  v=rd<u8>(s,0x4e); if(rd<u8>(p,balloon+3)!=v) { P vt=rd<P>(p); wr<u8>(p,balloon+3,v); pawn_v12d8(p,vt,v==0); }
  u8 b=rd<u8>(p,balloon+5); v=rd<u8>(s,0x4f);
  if(((b>>2)&1)!=v) { P vt=rd<P>(p); wr<u8>(p,balloon+5,((v<<2^b)&4)^b); pawn_v12d0(p,vt); b=rd<u8>(p,balloon+5); }
  u8 reliable=rd<u8>(p,vehicle+2); wr<u8>(p,balloon+5,((rd<u8>(s,0x50)<<3^b)&8)^b);
  v=rd<u8>(s,0x51); if(((reliable>>4)&1)!=v) { wr<u8>(p,vehicle+2,((v<<4^reliable)&0x10)^reliable); vehicle_changed(p); }
  b=rd<u8>(p,vehicle+1); wr<u8>(p,vehicle+1,b^((rd<u8>(s,0x52)<<5^b)&0x20));
  b=rd<u8>(p,vehicle+1); b=((rd<u8>(s,0x53)<<6^b)&0x40)^b; wr<u8>(p,vehicle+1,b);
  wr<u8>(p,vehicle+1,(rd<u8>(s,0x54)<<7)|(b&0x7f)); pawn_v1448(p,s);
}
}
