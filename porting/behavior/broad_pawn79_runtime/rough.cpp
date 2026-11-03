// Uncounted behavioral drafts of six native helpers. Address/offset accessors,
// containers, calls and virtual dispatch require native ABI and lifetime bindings.
#include <cstdint>
namespace broad_pawn79 {
using P = void*; using U64 = uint64_t; using I32 = int32_t;
extern P at_ptr(P, unsigned); extern U64 at_u64(P, unsigned); extern I32 at_i32(P, unsigned);
extern uint8_t at_u8(P, unsigned); extern P plus(P, int64_t);
extern P captured_vtable(P); extern P find_function_checked(P, uintptr_t name_global);
extern void process_event_v210(P captured_table, P object, P function, P params);
extern void free_140_free(P); extern void copy_bytes(P, P, int64_t);
// Array fields and temporary representations must preserve native size, aliases and initialization.
struct Bytes { P data; uint32_t count, capacity; };
extern void reserve_bytes_1406d9270(Bytes*, uint32_t, int);
extern bool same_address(P, P);
P get_related_142325510(P pawn) { return at_ptr(pawn, 0x1578); }
bool has_related_1423318f0(P pawn) { return at_ptr(pawn, 0x1578) != nullptr; }
void event_142a6f0a0(P object) {
  P table = captured_vtable(object);
  P function = find_function_checked(object, 0x1481b44e8);
  process_event_v210(table, object, function, nullptr);
}
void send_bytes_142a774e0(P object, Bytes* source) {
  Bytes args{nullptr, 0, 0};
  if (!same_address(&args, source)) {
    uint32_t count = source->count; P data = source->data;
    args.count = count;
    if (count != 0) {
      reserve_bytes_1406d9270(&args, count, 0);
      copy_bytes(args.data, data, static_cast<int64_t>(static_cast<int32_t>(count)));
    }
  }
  P table = captured_vtable(object);
  P function = find_function_checked(object, 0x1481b4aa8);
  process_event_v210(table, object, function, &args);
  if (args.data) free_140_free(args.data);
}
// Destination grow mutates data/capacity and is invoked after count increment.
struct Array { P data; I32 count, capacity; };
extern void grow_1407f1e50(Array*, I32 old_count);
extern void store_u64(P, unsigned, U64); extern void zero_u64(P, unsigned);
extern void tag_copy_assign(P dst, P src); // FGameplayTagContainer_CopyAssign
extern void copy_140682ce0(P dst, P src);
void gather_variants_141d50e00(P context, P wrap, Array* output) {
  P cursor = at_ptr(context, 0x48);
  P end = plus(cursor, static_cast<int64_t>(at_i32(context, 0x50)) * 0x48);
  for (; cursor != end; cursor = plus(cursor, 0x48)) {
    if (at_ptr(cursor, 0x30) != wrap) continue;
    I32 index = output->count; output->count = index + 1;
    if (output->capacity < index + 1) grow_1407f1e50(output, index);
    P dst = plus(output->data, static_cast<int64_t>(index) * 0x48);
    store_u64(dst, 0, at_u64(cursor, 0)); store_u64(dst, 8, at_u64(cursor, 8));
    zero_u64(dst, 0x10); zero_u64(dst, 0x18); zero_u64(dst, 0x20); zero_u64(dst, 0x28);
    tag_copy_assign(plus(dst, 0x10), plus(cursor, 0x10));
    store_u64(dst, 0x30, at_u64(cursor, 0x30));
    copy_140682ce0(plus(dst, 0x38), plus(cursor, 0x38));
  }
}
// Stack temporaries below are symbolic storage, not established native layouts.
struct Weak { uint32_t index; I32 serial; I32 epoch; };
struct Storage16 { U64 a, b; }; struct Name { uint32_t a, b; P allocation; U64 trailing; };
extern P find_variant_141d507d0(P wrap, U64 key);
extern void variant_v2a8(P variant, U64 value, Array* result, int zero);
extern void name_copy_142ebd9e0(Name*, P source); // destination starts zero
extern P weak_get(Weak*); extern P weak_resolve_142ed4d40(Name*);
extern void weak_set_142ee3b30(Weak*, P);
extern I32 atomic_read_epoch_147c4e988(); // native locked compare/exchange zero, then read
extern bool flag_148262b59(); extern P context_141ed2350();
extern P load_object_141b85960(Weak*, int one);
extern P class_1447976c0(); extern P class_14295e970();
extern bool class_chain_contains(P object_class, P target_class); // exact +0x38 index/+0x30 chain test
extern P object_class(P); // source dereferences *(longlong*)object->ClassPrivate; exact layout unresolved
extern P object_flags_subobject(P); // &object[1].ObjectFlags; UObject size/layout unresolved
extern P object_interface_v18(P object_plus_flags, P requested_class);
extern void increment_1481283bc(); extern I32 counter_1481283bc();
extern char state_140e51650(); extern I32 global_v90_1481282f8();
extern void name_to_string_1418c2930(Name*, Array*); extern void notify_141789dd0(P, Array*);
P select_material_141d85c80(P wrap, char mode, Array* variants, P notify_target) {
  Array collected{nullptr, 0, 0}; P selected;
  P cursor = variants->data;
  P end = plus(cursor, static_cast<int64_t>(variants->count) * 0x48);
  for (; cursor != end; cursor = plus(cursor, 0x48)) {
    if (at_ptr(cursor, 0x30) == wrap) {
      P variant = find_variant_141d507d0(wrap, at_u64(cursor, 0));
      if (variant) variant_v2a8(variant, at_u64(cursor, 8), &collected, 0);
    }
  }
  selected = collected.count < 1 ? plus(wrap, 0x5a8) : plus(collected.data, 0x40);
  Weak weak{static_cast<uint32_t>(at_u64(selected, 0)), at_i32(selected, 4), at_i32(selected, 8)};
  U64 source_name = at_u64(selected, 0x10);
  Storage16 auxiliary; copy_140682ce0(&auxiliary, plus(selected, 0x18));
  Name name{0,0,nullptr,0}; name_copy_142ebd9e0(&name, &source_name);
  // Each collected entry is 0x68 bytes; free both nested allocation fields in order.
  if (collected.count != 0) {
    P item = plus(collected.data, 0x18); I32 remaining = collected.count;
    do {
      P second = at_ptr(item, 0x40); if (second) free_140_free(second);
      P first = at_ptr(item, 0); if (first) free_140_free(first);
      item = plus(item, 0x68); --remaining;
    } while (remaining != 0);
  }
  if (collected.data) free_140_free(collected.data);
  P result = nullptr;
  if (name.a != 0 || name.b != 0) {
    P object = nullptr;
    if (at_u8(context_141ed2350(), 0x1d5) == 0) {
      object = weak_get(&weak);
      if (!object) {
        // The source repeats the name-empty test before the weak cache refresh.
        if (name.a != 0 || name.b != 0) {
          I32 epoch = atomic_read_epoch_147c4e988();
          if (epoch != weak.epoch || static_cast<I32>(weak.index) != -1) {
            P resolved = weak_resolve_142ed4d40(&name);
            weak_set_142ee3b30(&weak, resolved);
            if (resolved || !flag_148262b59()) weak.epoch = atomic_read_epoch_147c4e988();
            object = weak_get(&weak);
          }
        }
      }
      if (object) { P cls = class_1447976c0(); if (!class_chain_contains(object_class(object), cls)) object = nullptr; }
      if (!object && notify_target) {
        increment_1481283bc();
        char state = state_140e51650();
        bool notify = state != 4;
        if (!notify) { I32 threshold = global_v90_1481282f8(); notify = counter_1481283bc() <= threshold; }
        if (notify) {
          name_to_string_1418c2930(&name, &collected);
          notify_141789dd0(notify_target, &collected);
          if (collected.data) free_140_free(collected.data);
        }
      }
    } else {
      object = load_object_141b85960(&weak, 1);
    }
    if (object) {
      P requested = class_14295e970();
      P iface = object_interface_v18(object_flags_subobject(object), requested);
      P iface_class = iface ? class_14295e970() : nullptr;
      if (iface && class_chain_contains(at_ptr(iface, 0x10), iface_class)) {
        if (mode == 0) result = at_ptr(iface, 0x38);
        else if (mode == 1) result = at_ptr(iface, 0x28);
        else if (mode == 2) result = at_ptr(iface, 0x30);
      }
    }
  }
  if (name.allocation) free_140_free(name.allocation);
  return result;
}
}
