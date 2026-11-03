// Rough behavioral proposals from 12.60 native code. Not production definitions.
// Adapters name observed operations; they do not assert recovered engine APIs.
#include <cstdint>

namespace supplydrop92 {
struct Object;
struct Class;
struct World;
struct Drop;
struct Vec3 { float x, y, z; };
struct ObjectArray { Object** data; int32_t count; int32_t capacity; };

World* world_vfunc_150(Drop*);
Object* world_field_128(World*);
Class* class_1428ee970();
Class* class_142b90cb0();
Class* class_player_state_athena();
// Implements the exact class-depth/ancestor-slot check observed at each call site.
bool object_has_ancestor(Object*, Class*);
void spawn_data_141949a30(Object*, Drop*, float*, float*);
void speed_data_141949b10(Object*, Drop*, float*, float*);
void collect_actors_144212e10(World*, Class*, ObjectArray*);
bool actor_predicate_142ecc620(Object*);
float read_float(Object*, unsigned offset);
Object* read_object(Object*, unsigned offset);
void free_array_storage(Object**);
void* drop_field_790(Drop*);
float drop_spawn_offset_z_834(Drop*);
Vec3 ground_location_141eede00(World*, Drop*, const Vec3*, float, float, void*);
void notify_player_state_141a4cf10(Object*, void*);
void* drop_field_address_410(Drop*);

// 0x1419142d0: the helper receives the world, input and trace parameters;
// its effects and failure semantics remain unresolved.
Vec3 FindGroundLocationAt(Drop* self, const Vec3* input) {
    World* world = world_vfunc_150(self);
    Vec3 result = ground_location_141eede00(world, self, input, 10000.0f, -5000.0f,
                                             drop_field_790(self));
    result.z += drop_spawn_offset_z_834(self);
    return result;
}

// 0x1419189c0. Only the first collected actor is inspected; do not search later ones.
void GetSupplyDropSpawnData(Drop* self, float* first, float* second) {
    World* world = world_vfunc_150(self);
    if (world) {
        Object* mode = world_field_128(world);
        if (mode && object_has_ancestor(mode, class_1428ee970())) {
            spawn_data_141949a30(mode, self, first, second);
            return;
        }
        ObjectArray actors{};
        collect_actors_144212e10(world, class_142b90cb0(), &actors);
        if (actors.count > 0 && actors.data[0]) {
            Object* actor = actors.data[0];
            if (object_has_ancestor(actor, class_142b90cb0()) &&
                actor_predicate_142ecc620(actor)) {
                *first = read_float(actor, 0xb98);
                *second = read_float(actor, 0xb9c);
                if (actors.data) free_array_storage(actors.data);
                return;
            }
        }
        if (actors.data) free_array_storage(actors.data);
    }
    *first = 28000.0f;
    *second = 32000.0f;
}

// 0x141918b00: the two output pointers may alias; preserve write order.
void GetSupplyDropSpeedData(Drop* self, float* first, float* second) {
    World* world = world_vfunc_150(self);
    if (world) {
        Object* mode = world_field_128(world);
        if (mode && object_has_ancestor(mode, class_1428ee970())) {
            speed_data_141949b10(mode, self, first, second);
            return;
        }
    }
    *first = 280.0f;
    *second = 500.0f;
}

// 0x141922280: input's +0x220 object is checked as a player state.
void SpawningLootOnDestruction(Drop* self, Object* input) {
    if (input) {
        Object* state = read_object(input, 0x220);
        if (state && object_has_ancestor(state, class_player_state_athena()))
            notify_player_state_141a4cf10(state, drop_field_address_410(self));
    }
}
} // namespace supplydrop92

namespace supplydrop92 {
struct TimerManager;
struct TimerHandle;
struct Delegate;
struct WeakObject;
struct Listener;
struct ListenerEntry { Listener* listener; int32_t marker; /* native stride 16 */ };
struct ListenerRegistry {
    ListenerEntry* entries;
    int32_t count;
    int32_t capacity;
    int32_t compact_threshold;
    int32_t invocation_depth;
};
extern ListenerRegistry& storm_registry_1478c2688;
struct PositionBits { uint32_t word[3]; };
struct DelegateCopy { void* pointer; int32_t active; };
struct LocalWeakState; // separate local weak/inline object, not a delegate copy

PositionBits position_from_root_or_global_148218630(Drop*); // self +0x130, root +0x1d0
bool mode_test_14194e990(Object*, const PositionBits*, int zero);
TimerManager* timer_manager_140f95ae0(Drop*);
TimerHandle* timer_handle_880(Drop*);
TimerHandle* timer_handle_888(Drop*);
bool timer_exists_144621fb0(TimerManager*, TimerHandle*);
void clear_timer_144627760(TimerManager*, TimerHandle*);
void zero_timer_handle(TimerHandle*);
float read_drop_float_870(Drop*);
// Constructor: allocate 0x30, install initial vtable, initialize weak self at +8,
// copy 16 bytes of callback member representation (only its first 12 initialized),
// store 142bad740() result at +0x28, install final vtable. Null allocation allowed.
Delegate* construct_storm_delegate_14191c710(Drop*);
void clone_delegate_vfunc_40(Delegate*, DelegateCopy*);
void initialize_local_weak_142ee3b30(LocalWeakState*, Object*);
void set_timer_1446278d0(TimerManager*, TimerHandle*, DelegateCopy*, float rate,
                         bool loop, float first_delay, LocalWeakState*);
void destroy_local_weak_if_engaged(LocalWeakState*);
void delegate_copy_vfunc_38(DelegateCopy*);
void* realloc_zero(void*);
void memory_free(void*);
void delegate_original_vfunc_38(Delegate*);
LocalWeakState* local_weak_storage(); // conceptual independent stack storage

// 0x14191c450. Delegate, cloned delegate and local weak-state destruction are
// deliberately exposed separately: call-frame and inline-storage ABI unresolved.
void OnCheckForBeingInStorm(Drop* self) {
    World* world = world_vfunc_150(self);
    if (!world) return;
    Object* mode = world_field_128(world);
    if (!mode || !object_has_ancestor(mode, class_1428ee970())) return;
    PositionBits position = position_from_root_or_global_148218630(self);
    if (mode_test_14194e990(mode, &position, 0)) return;

    TimerManager* manager = timer_manager_140f95ae0(self);
    TimerHandle* prior = timer_handle_880(self);
    if (timer_exists_144621fb0(manager, prior)) clear_timer_144627760(manager, prior);
    zero_timer_handle(prior);
    float rate = read_drop_float_870(self);
    manager = timer_manager_140f95ae0(self);
    Delegate* original = construct_storm_delegate_14191c710(self);
    DelegateCopy copy{nullptr, 0};
    if (original) clone_delegate_vfunc_40(original, &copy);
    LocalWeakState* local = local_weak_storage();
    initialize_local_weak_142ee3b30(local, nullptr);
    set_timer_1446278d0(manager, timer_handle_888(self), &copy, rate, false, -1.0f, local);
    destroy_local_weak_if_engaged(local);
    if (copy.active != 0 && copy.pointer) {
        delegate_copy_vfunc_38(&copy);
        if (copy.pointer) copy.pointer = realloc_zero(copy.pointer);
        copy.active = 0;
    }
    if (copy.pointer) memory_free(copy.pointer);
    if (original) {
        delegate_original_vfunc_38(original);
        void* remaining = realloc_zero(original);
        if (remaining) memory_free(remaining);
    }
}

struct NameBits { uint32_t comparison; uint32_t number; };
NameBits drop_name_848(Drop*);
uint32_t drop_word_844(Drop*);
bool fname_comparison_equals_zero(uint32_t);
bool listener_vfunc_50(Listener*, uint32_t, NameBits);
bool listener_vfunc_20(Listener*);
void registry_remove_140690780(ListenerRegistry&, int32_t index, int count, bool allow_shrink);
void registry_compact_1406911b0(ListenerRegistry&);
void destroy_actor_143f6c200(Drop*, bool, bool);

// 0x14191c710. Registry is re-read around callbacks because those calls may mutate it.
void OnDespawnDueToStorm(Drop* self) {
    NameBits name = drop_name_848(self);
    bool equals_none = fname_comparison_equals_zero(name.comparison);
    if (drop_name_848(self).number != 0 || !equals_none) {
        // Native dereferences world without a null guard on this path.
        Object* mode = world_field_128(world_vfunc_150(self));
        if (mode && object_has_ancestor(mode, class_1428ee970())) {
            ListenerRegistry& r = storm_registry_1478c2688;
            ++r.invocation_depth;
            name = drop_name_848(self);
            uint32_t word = drop_word_844(self);
            bool needs_cleanup = false;
            for (int64_t i = static_cast<int64_t>(r.count) - 1; i >= 0; --i) {
                ListenerEntry& entry = r.entries[i];
                if (entry.marker == 0 || !entry.listener ||
                    !listener_vfunc_50(entry.listener, word, name)) needs_cleanup = true;
            }
            int32_t count_before_cleanup = r.count;
            --r.invocation_depth;
            if (needs_cleanup && r.invocation_depth < 1) {
                int32_t i = 0;
                while (i < r.count) {
                    ListenerEntry& entry = r.entries[i];
                    if (entry.marker == 0 || !entry.listener || listener_vfunc_20(entry.listener))
                        registry_remove_140690780(r, i, 1, true);
                    else
                        ++i;
                }
                r.compact_threshold = r.count * 2;
                if (r.compact_threshold < 3) r.compact_threshold = 2;
                if (r.compact_threshold < count_before_cleanup && r.capacity != r.count)
                    registry_compact_1406911b0(r);
            }
        }
    }
    destroy_actor_143f6c200(self, false, true);
}
} // namespace supplydrop92
