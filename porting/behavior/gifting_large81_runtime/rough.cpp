// Partial source recovery of 0x14259af20, NOT a complete body or ABI.
// Only the initial gate and the repeated 0x50-byte queue removal are lifted.
#include <cstdint>
#include <cstddef>
#include <cstring>
using u8 = uint8_t; using u32 = uint32_t; using u64 = uint64_t;
using i32 = int32_t; using i64 = int64_t;
template<class T> T& at(void* p, size_t n) { return *reinterpret_cast<T*>((u8*)p+n); }
extern void* global_148351eb8;
extern double seconds_per_counter_1481d47e8;
extern u8 verbosity_1480e6bf8, verbosity_1481348e0;
extern void* call_14139ae30(u64), *call_144217e60(void*), *call_14468f240(void*);
extern void* class_142a3d9f0(), *call_14447f200(void*);
extern void initialize_144680860(void*);
// With omitted native arguments, these adapters explicitly own the actual call ABI
// and its interaction with the persistent stack slot; no counter value is invented.
extern void query_counter_missing_argument(u64* persistent_slot);
extern void query_counter_explicit(u64* persistent_slot);
extern void format_142bafb80(void** output, const wchar_t*, i32);
extern void call_14102e390(void*);
extern void free_memory(void*);
extern void log_lost_pawn();
extern void log_gc_missing_varargs();
extern void virtual_360(void*);
extern u64 call_142c98540();
extern void array_remove_141db8470(void*, i32, i32, i32);
extern void* resolve_141aacc90(void*);
extern void controller_virtual_0(void*), controller_virtual_8_missing_argument(void*);

// Storage remains caller-owned because it is reused in the unrecovered continuation.
struct GateFrame {
  alignas(16) u8 stack_358[0x2a]; // initializer writes unknown contents, including +0x28/+0x29
  u64 stack_4b8;                  // incoming contents are not initialized by this lift
  void* world; void* owner; void* pawn; void* typed;
};
enum class GateResult { exit, continue_before_queue_dispatch, continue_at_b5ad };
GateResult gate_14259af20(void* self, GateFrame& f) {
  void* engine = global_148351eb8;
  f.owner = call_14139ae30(at<u64>(engine,0x7c0));
  if (!f.owner) return GateResult::exit;
  f.world = call_144217e60(f.owner);
  if (!f.world) return GateResult::exit;
  void* typed = call_14468f240(f.world);
  f.typed = typed;
  if (!typed) return GateResult::exit;
  void* cls = class_142a3d9f0();
  void* type = at<void*>(typed,0x10);
  i32 depth = at<i32>(cls,0x38);
  if (at<i32>(type,0x38) < depth ||
      at<void*>(at<void*>(type,0x30), (i64)depth*8) != (u8*)cls+0x30)
    return GateResult::exit;
  f.pawn = call_14447f200(typed);
  initialize_144680860(f.stack_358);
  f.stack_358[0x29] |= 2;
  f.stack_358[0x28] = 1;
  query_counter_missing_argument(&f.stack_4b8);
  if ((double)at<float>(self,0xac) < (double)(i64)f.stack_4b8 * seconds_per_counter_1481d47e8 + 16777216.0) {
    void* message; // formatter constructs output
    format_142bafb80(&message,L"PSO Testpass currently in SpawnAssets mode. Items spawned so far: %d",at<i32>(self,0x7c));
    call_14102e390(self);
    if (message) free_memory(message);
    query_counter_missing_argument(&f.stack_4b8);
    at<float>(self,0xac) = (float)((double)(i64)f.stack_4b8 * seconds_per_counter_1481d47e8 + 16777216.0 + (double)at<float>(self,0xa8));
  }
  if (!f.pawn) {
    if (verbosity_1480e6bf8 > 4) log_lost_pawn();
    virtual_360(f.owner);
    at<u32>(self,0x94)=8;
    return GateResult::exit;
  }
  if (at<i32>(self,0x84) < at<i32>(self,0x7c)) {
    at<i32>(self,0x84)=at<i32>(self,0x7c)+at<i32>(self,0x78);
    // The vtable target is loaded before the call that produces the fourth argument.
    engine=global_148351eb8;
    void* gc_receiver=(u8*)engine+0x28;
    u64 gc_vtable=at<u64>(engine,0x28);
    u64 gc_arg=call_142c98540();
    // gc_vtable is captured separately because the call may mutate the receiver.
    extern void gc_virtual_8_captured(u64,void*,void*,const wchar_t*,u64);
    gc_virtual_8_captured(gc_vtable,gc_receiver,f.world,L"obj gc",gc_arg);
    if (verbosity_1481348e0 > 4) log_gc_missing_varargs();
  }
  query_counter_explicit(&f.stack_4b8);
  if ((double)(i64)f.stack_4b8 * seconds_per_counter_1481d47e8 + 16777216.0 <= (double)at<float>(self,0xb8))
    return GateResult::continue_at_b5ad;
  return GateResult::continue_before_queue_dispatch;
}

// Shared expanded sequence at source lines 170-211, 410-451 and 1450-1491.
// Caller supplies existing stack storage; bytes 0x4c..0x4f are not initialized.
void* pop_resolve_0x50(void* array, u8* temporary) {
  u8* data=at<u8*>(array,0);
  i64 count=at<i32>(array,8);
  u8* item=data-0x50+count*0x50;
  at<u64>(temporary,0)=at<u64>(item,0);
  at<u64>(temporary,8)=at<u64>(item,8);
  at<u64>(temporary,0x18)=at<u64>(item,0x18);
  at<u64>(temporary,0x10)=at<u64>(item,0x10);
  at<u64>(temporary,0x28)=at<u64>(item,0x28);
  at<u64>(temporary,0x20)=at<u64>(item,0x20);
  at<u64>(temporary,0x30)=at<u64>(item,0x30);
  at<u64>(item,0x30)=0;
  at<u64>(item,0x28)=0;
  at<u64>(temporary,0x38)=at<u64>(item,0x38);
  at<u64>(item,0x38)=0;
  at<u32>(temporary,0x40)=at<u32>(item,0x40);
  at<u32>(temporary,0x44)=at<u32>(item,0x44);
  at<u64>(item,0x40)=0;
  at<u32>(temporary,0x48)=at<u32>(item,0x48);
  array_remove_141db8470(array,at<i32>(array,8)-1,1,1);
  void* result=resolve_141aacc90(temporary);
  void* allocation=at<void*>(temporary,0x38);
  if (allocation) free_memory(allocation);
  void* controller=at<void*>(temporary,0x30);
  if (controller && --at<i32>(controller,8)==0) {
    controller_virtual_0(controller);
    if (--at<i32>(controller,12)==0) controller_virtual_8_missing_argument(controller);
  }
  return result;
}

// World-item queue differs from the first three: validate before cleanup and
// destroy the shared pair through 0x140682f30 rather than the inlined release.
extern u8 predicate_1425791e0(void*);
extern void destroy_pair_140682f30(void*);
void* pop_world_item_0x50(void* array, u8* temporary) {
  u8* item=at<u8*>(array,0)-0x50+(i64)at<i32>(array,8)*0x50;
  at<u64>(temporary,0)=at<u64>(item,0);
  at<u64>(temporary,8)=at<u64>(item,8);
  at<u64>(temporary,0x10)=at<u64>(item,0x10);
  at<u64>(temporary,0x18)=at<u64>(item,0x18);
  at<u64>(temporary,0x20)=at<u64>(item,0x20);
  at<u64>(temporary,0x28)=at<u64>(item,0x28);
  at<u64>(temporary,0x30)=at<u64>(item,0x30);
  at<u64>(item,0x30)=0;
  at<u64>(item,0x28)=0;
  at<u64>(temporary,0x38)=at<u64>(item,0x38);
  at<u64>(item,0x38)=0;
  at<u32>(temporary,0x40)=at<u32>(item,0x40);
  at<u32>(temporary,0x44)=at<u32>(item,0x44);
  at<u64>(item,0x40)=0;
  at<u32>(temporary,0x48)=at<u32>(item,0x48);
  array_remove_141db8470(array,at<i32>(array,8)-1,1,1);
  void* item_object=resolve_141aacc90(temporary);
  if (!item_object || !predicate_1425791e0(item_object)) item_object=nullptr;
  if (at<void*>(temporary,0x38)) free_memory(at<void*>(temporary,0x38));
  destroy_pair_140682f30(temporary+0x28);
  return item_object;
}

// The world-item branch constructs this entry only after its name passes the
// processing predicate. The caller retains both native stack regions and must
// continue through the spawn/copy and both destructors in their native order.
extern void entry_construct_1420237f0(void*,void*,int);
extern void entry_update_142038ae0(void*,int,int);
extern void entry_gifting_1420387b0(void*,void*);
extern void entry_update_1420385d0(void*,int);
extern void entry_update_142022bf0(void*,int,int);
extern u32 entry_update_142029d10(void*);
u32 prepare_world_item_entry(void* entry, void* item_object, void* gifting_stack_478) {
  entry_construct_1420237f0(entry,item_object,1);
  entry_update_142038ae0(entry,0,0);
  at<u64>(gifting_stack_478,0)=0;
  at<u64>(gifting_stack_478,8)=0;
  at<u64>(gifting_stack_478,0x20)=0;
  at<u64>(gifting_stack_478,0x10)=0;
  at<u64>(gifting_stack_478,0x18)=0;
  entry_gifting_1420387b0(entry,gifting_stack_478);
  entry_update_1420385d0(entry,1);
  entry_update_142022bf0(entry,3,1);
  entry_update_142022bf0(entry,9,1);
  return entry_update_142029d10(entry);
}

// Common continuation at LAB_14259b5ad. Scratch storage is supplied by the
// caller, since the virtual's write extent and original stack aliasing are ABI.
extern void virtual_660(void* typed, void* output);
extern void call_142111830(void* typed, void* vector);
void position_continuation_14259b5ad(void* typed, void* stack_368, void* stack_378) {
  virtual_660(typed,stack_368);
  at<u32>(stack_378,8)=at<u32>(stack_368,8); // uStack_370 = uStack_360
  at<u32>(stack_378,0)=at<u32>(stack_368,0);
  at<float>(stack_378,4)=at<float>(stack_368,4)+5.0f;
  call_142111830(typed,stack_378);
}
