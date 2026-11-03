// Uncounted source lift; declarations below are unresolved ABI adapters, not implementations.
#include <cstdint>
#include <cstring>
namespace creative_supply76 {
using i32 = int32_t; using u32 = uint32_t; using u64 = uint64_t;
struct Object; struct Controller; struct Type; struct Ref; struct World;
struct Array { void* data; i32 num, max; };
struct Item { i32 quantity; u32 padding; Object* object; };
struct Pair { unsigned char bytes[20]; };
struct Weak { unsigned char bytes[24]; };
struct SpawnParams { unsigned char bytes[40]; };
struct ItemSpawn { unsigned char bytes[336]; };
struct DropSpawn { unsigned char bytes[336]; };
struct Transform { unsigned char bytes[48]; };
extern World* world_v150(Controller*); extern bool fun_1419c3880();
extern u64 unresolved_gate_stack_458_after_1419c3880();
extern Array& cached(Controller*); extern bool log_enabled(); extern void log_attempt();
extern void log_convert_failed(); extern void log_convert_success(Object*, Pair&);
extern void log_chest(); extern void log_drop();
extern Type* fun_1446acb00(); extern Type* fun_1427d5b40(); extern Type* fun_142739440(); extern Type* fun_142b5be40();
// This implements the observed class-depth/ancestor-slot check; null behavior must remain raw.
extern bool ancestor_check(void* object_or_class, Type* type);
extern void* class_cached_19c(void*); extern void class_v390(void*);
extern bool fun_14214bd70(Object*, void*, Controller*, int);
extern void fun_144680860(SpawnParams&); extern void set_spawn_flags(SpawnParams&, unsigned char, unsigned char);
extern Object* fun_1442e74f0(World*, void*, Object*, const void*);
extern void init_weak(Weak&, Object*); extern void fun_142ebd0a0_and_142ebd9e0(Weak&, Object*);
extern void fun_142ee3b30(Weak&, Object*); extern u32 atomic_global_147c4e988();
extern void weak_set_global(Weak&, u32); extern Pair fun_141f7ce30(Weak&, i32);
extern void* weak_temporary_allocation(Weak&); extern void array_grow_pairs(Array&, i32 old_num);
extern Object* fun_141f88f00(Pair&);
extern Object* fun_141f88f00_missing_args();
extern void* virtual_268(Object*, void* output); extern void virtual_268_missing_output(Object*);
extern void fun_142c40540(void*); extern void fun_142c40540_missing_args();
extern Ref* unresolved_log_ref_3a0(); extern Ref* unresolved_log_ref_388();
extern i32 atomic_decrement_strong(Ref*); extern i32 atomic_decrement_weak(Ref*);
extern void ref_virtual_0(Ref*); extern void ref_virtual_8(Ref*);
extern void log_converted_names();
extern Array& drop_items(Object*); extern Array& chest_items(Object*); extern void grow_append(Array&, i32 count);
extern void fun_142c5c060(void* zeroed_out, Object** spawned_alias);
extern u64 raw_first_8(Object*); extern u32 raw_next_4(Object*);
extern u64 global_147c31ca0(); extern u32 global_147c31ca8();
extern u32 unresolved_stack_440(); extern u32 unresolved_stack_43c();
extern unsigned char unresolved_stack_19e_after_constructor();
extern void fun_143f71060(Object*, Transform&, int, int); extern void virtual_880(Object*, int, int, int); extern void virtual_888(Object*, int);
extern void free_memory(void*); extern void resize_cached_zero(Array&);
extern i32 item_v390(Object*, int); extern void fun_1420237f0(ItemSpawn&, Object*, i32, int);
extern void fun_1420237c0(DropSpawn&);
extern u64 global_148218640(); extern u32 global_148218648();
// These fields are separate stack objects, not asserted to fit inside DropSpawn.
struct DropCallFrame { World* prefix; DropSpawn body; u64 input8; u32 input4; u64 global8; u32 global4; u64 zero1, zero2; Controller* owner; uint16_t flags304; unsigned char flags; };
extern Object* fun_142029890_frame(DropCallFrame&);
extern void fun_14203cda0(Object*, Object*, int);
extern void fun_1419844f0(void*);
extern double performance_seconds(); extern double cooldown_global(); extern double& last_spawn(Controller*);
extern Object* fun_141991c10(Controller*, u64); extern bool bit_9fc(Object*); extern Object* fun_141ed2350(); extern bool byte_c9(Object*);
extern void* class_plus_30(Type*);

static void release_log_ref(Ref* ref) {
  if (ref && atomic_decrement_strong(ref) == 1) {
    ref_virtual_0(ref);
    if (atomic_decrement_weak(ref) == 1) ref_virtual_8(ref);
  }
}
static void write32(void* dst, u32 v) { std::memcpy(dst, &v, 4); }
static void write64(void* dst, u64 v) { std::memcpy(dst, &v, 8); }
void implementation(Controller* self, Object* arg, void* position) {
  World* world = world_v150(self);
  if (world) {
    u64 gate = 0;
    bool allowed = fun_1419c3880();
    if (allowed) gate = unresolved_gate_stack_458_after_1419c3880();
    if (allowed && gate) {
      if (log_enabled()) log_attempt();
      Array& items = cached(self);
      if (items.num == 0) return; // bypasses cache clearing
      Type* base = position ? fun_1446acb00() : nullptr;
      if (position && base && ancestor_check(position, base)) {
        void* cached_class = class_cached_19c(position);
        if (!cached_class) { class_v390(position); cached_class = class_cached_19c(position); }
        if (!fun_14214bd70(arg, cached_class, self, 0)) return; // bypasses clearing
        SpawnParams params; fun_144680860(params); set_spawn_flags(params, 6, 1);
        u64 spawn_location[2] = {0, 0};
        base = fun_1446acb00();
        void* cast_position = base && ancestor_check(position, base) ? position : nullptr;
        Object* spawned = fun_1442e74f0(world, cast_position, arg, spawn_location);
        Object* drop = nullptr; Object* chest = nullptr;
        if (spawned && ancestor_check(spawned, fun_1427d5b40())) drop = spawned;
        if (spawned && ancestor_check(spawned, fun_142739440())) chest = spawned;
        Array pairs{nullptr, 0, 0};
        for (i32 i = 0; i < items.num; ++i) {
          Item* entry = static_cast<Item*>(items.data) + i;
          Object* item = entry->object;
          Weak weak; init_weak(weak, item);
          if (item) {
            fun_142ebd0a0_and_142ebd9e0(weak, item); fun_142ee3b30(weak, item);
            weak_set_global(weak, atomic_global_147c4e988());
          }
          entry = static_cast<Item*>(items.data) + i;
          Pair pair = fun_141f7ce30(weak, entry->quantity);
          if (void* allocation = weak_temporary_allocation(weak)) free_memory(allocation);
          i32 old = pairs.num++;
          if (pairs.max < pairs.num) array_grow_pairs(pairs, old);
          static_cast<Pair*>(pairs.data)[old] = pair;
          entry = static_cast<Item*>(items.data) + i;
          if (!entry->object || !fun_141f88f00_missing_args()) {
            if (log_enabled()) log_convert_failed();
          } else if (log_enabled()) {
            Object* converted = fun_141f88f00(pair);
            entry = static_cast<Item*>(items.data) + i;
            unsigned char name_output[16];
            void* name = virtual_268(converted, name_output);
            fun_142c40540(name);
            virtual_268_missing_output(entry->object);
            fun_142c40540_missing_args();
            log_converted_names();
            release_log_ref(unresolved_log_ref_3a0());
            release_log_ref(unresolved_log_ref_388());
          }
        }
        Object* target = nullptr;
        if (drop && items.num > 0) { if (log_enabled()) log_drop(); target = drop; }
        else if (chest && items.num > 0) { if (log_enabled()) log_chest(); target = chest; }
        if (target) {
          Array& destination = target == drop ? drop_items(target) : chest_items(target);
          if (pairs.num != 0) {
            i32 old = destination.num;
            if (destination.max < pairs.num + old) { grow_append(destination, pairs.num); old = destination.num; }
            std::memcpy(static_cast<unsigned char*>(destination.data) + old * 20, pairs.data, pairs.num * 20);
            destination.num += pairs.num;
          }
          unsigned char zeroed[16] = {}; Object* alias = drop;
          fun_142c5c060(zeroed, &alias);
          Transform transform;
          write64(transform.bytes + 16, raw_first_8(arg));
          write32(transform.bytes + 24, raw_next_4(arg));
          write32(transform.bytes + 28, 0);
          write64(transform.bytes + 32, global_147c31ca0());
          write32(transform.bytes + 40, global_147c31ca8());
          write32(transform.bytes + 44, 0);
          write64(transform.bytes, reinterpret_cast<u64>(alias));
          write32(transform.bytes + 8, unresolved_stack_440());
          write32(transform.bytes + 12, unresolved_stack_43c());
          fun_143f71060(target, transform, 0, 0);
          virtual_880(target, 1, -1, 0); virtual_888(target, 1);
        }
        if (pairs.data) free_memory(pairs.data);
      } else {
        for (i32 i = 0; i < items.num; ++i) {
          Item* entry = static_cast<Item*>(items.data) + i;
          Object* item = entry->object;
          if (item && !ancestor_check(item, fun_142b5be40())) item = nullptr;
          i32 original = entry->quantity, remaining = original;
          if (remaining > 0 && item) do {
            i32 limit = item_v390(item, 0);
            i32 chunk = limit <= remaining ? limit : remaining;
            ItemSpawn item_spawn; fun_1420237f0(item_spawn, item, chunk, 0);
            DropCallFrame frame;
            frame.prefix = world;
            fun_1420237c0(frame.body);
            frame.input4 = raw_next_4(arg);
            frame.input8 = raw_first_8(arg);
            frame.flags = unresolved_stack_19e_after_constructor() | 3;
            frame.global4 = global_148218648();
            frame.zero1 = 0; frame.zero2 = 0;
            frame.global8 = global_148218640();
            frame.flags304 = 0x304;
            frame.owner = self;
            Object* spawned = fun_142029890_frame(frame);
            i32 basis = chunk != 0 ? remaining : original;
            if (spawned) {
              unsigned char toss_input[12];
              write32(toss_input + 8, raw_next_4(arg));
              write64(toss_input, raw_first_8(arg));
              fun_14203cda0(spawned, reinterpret_cast<Object*>(toss_input), 0);
            }
            fun_1419844f0(&frame.body); fun_1419844f0(&item_spawn);
            remaining = basis - chunk;
          } while (remaining > 0);
        }
      }
    }
  }
  Array& items = cached(self); items.num = 0;
  if (items.max != 0) resize_cached_zero(items);
}

bool validate(Controller* self, Object* arg, void* position) {
  (void)arg;
  double now = performance_seconds(); double threshold = cooldown_global();
  double previous = last_spawn(self);
  last_spawn(self) = performance_seconds() + 16777216.0;
  Type* type = fun_1427d5b40();
  Type* base = position ? fun_1446acb00() : nullptr;
  void* cast = position && base && ancestor_check(position, base) ? position : nullptr;
  void* class_tail;
  if (type && ancestor_check(cast, type)) class_tail = class_plus_30(type);
  else {
    type = fun_142739440();
    base = position ? fun_1446acb00() : nullptr;
    cast = position && base && ancestor_check(position, base) ? position : nullptr;
    if (!type) return false;
    class_tail = class_plus_30(type);
    if (!ancestor_check(cast, type)) return false;
  }
  u64 hidden = (reinterpret_cast<u64>(class_tail) & ~u64(255)) | 1;
  Object* mode = fun_141991c10(self, hidden);
  return bit_9fc(mode) && byte_c9(fun_141ed2350()) && threshold <= (now + 16777216.0) - previous;
}
}
