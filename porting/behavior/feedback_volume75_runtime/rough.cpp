// Readable, unlinked lift of native 0x1424c21f0 and 0x1419c3880.
// Adapter declarations denote unresolved native calls, globals, and virtual ABI.
#include <cstddef>
#include <cstdint>

namespace feedback_volume75 {
using u8 = std::uint8_t;
using i32 = std::int32_t;
using u64 = std::uint64_t;
using ptr = std::uintptr_t;
template<class T> T& field(ptr p, std::size_t offset) {
    return *reinterpret_cast<T*>(p + offset);
}
extern ptr call_1424ab180();
extern ptr virtual_ptr(ptr object, std::size_t slot);
extern ptr virtual_ptr_arg(ptr object, std::size_t slot, ptr argument);
extern float virtual_float(ptr object, std::size_t slot);
extern void virtual_void(ptr object, std::size_t slot);
extern void virtual_void_int(ptr object, std::size_t slot, int argument);
extern ptr call_1424aaf90(ptr, ptr);
extern void call_1407096c0(ptr map, i32* index, u64 key);
extern float call_14231f230(ptr actor, ptr key);
extern bool call_1424b20c0(ptr owner, ptr value);
extern int native_rand();
extern void call_1424b6f30(ptr owner, const void* payload, u8 flag);
extern void call_1423524a0(ptr actor);
extern i32 global_i32(ptr address);
extern ptr call_141c74820();
extern ptr call_141c7d420(ptr, ptr);
extern ptr AFortGameStateAthena_StaticClass();
extern bool call_14199e2c0(ptr);
extern ptr call_14297b320();
extern ptr call_14237f3d0(ptr, void*);
extern u8 call_141eeab20(ptr, ptr);
extern ptr global_ptr_145a95888(); // address of the native vtable, not its first entry
extern void FMemory_Free(ptr);

// Stack payload at native stack offsets 0x90..0x64, relative ordering retained.
struct DispatchPayload {
    ptr actor3, actor4;
    u64 kind;
    u64 key1, key2;
    float magnitude;
    u8 flag;
};

void FeedbackDispatch(ptr self, ptr descriptor, ptr actor3, ptr actor4,
                      float magnitude, u8 flag, u8 dispatchFlag) {
    ptr config = call_1424ab180();
    if (!config) {
        if (!actor3) return;
        ptr fallbackKind = virtual_ptr(actor3, 0xc70);
        if (!fallbackKind) return;
        ptr rules = call_1424aaf90(fallbackKind, descriptor);
        if (!rules) return;
        i32 index;
        call_1407096c0(self + 0x2b0, &index, field<u64>(descriptor, 8));
        ptr entry = index == -1 ? 0 : field<ptr>(self, 0x2b0) + std::intptr_t(index) * 0x14;
        ptr previous = entry ? entry + 8 : 0;
        float actorTime = call_14231f230(actor3, descriptor + 8);
        ptr world = virtual_ptr(self, 0x150);
        float now = field<float>(world, 0x540);
        if (previous) {
            float previousTime = field<float>(previous, 0);
            float cooldown = virtual_float(rules, 0);
            if (now - previousTime < cooldown) return;
        }
        if (actorTime != -1.0f) {
            float cooldown = virtual_float(rules, 8);
            if (now - actorTime < cooldown) return;
        }
        DispatchPayload payload;
        payload.actor3 = actor3;
        payload.actor4 = actor4;
        payload.kind = fallbackKind;
        payload.key1 = field<u64>(descriptor, 8);
        payload.key2 = field<u64>(descriptor, 16);
        payload.magnitude = magnitude <= 0.0f ? 0.0f : magnitude;
        payload.flag = flag;
        call_1424b6f30(self, &payload, dispatchFlag);
        return;
    }
    ptr value3 = actor3 ? field<ptr>(actor3, 0x250) : 0;
    ptr value4 = actor4 ? field<ptr>(actor4, 0x250) : 0;
    if (field<u8>(config, 0x2c) == 0 && call_1424b20c0(self, value3)) return;
    if (field<u8>(config, 0x2c) == 1 && call_1424b20c0(self, value4)) return;
    i32 index;
    call_1407096c0(self + 0x2b0, &index, field<u64>(descriptor, 8));
    ptr entry = index == -1 ? 0 : field<ptr>(self, 0x2b0) + std::intptr_t(index) * 0x14;
    ptr previous = entry ? entry + 8 : 0;
    float actorTime = -1.0f;
    if (field<u8>(config, 0x2c) == 0) {
        if (actor3) actorTime = call_14231f230(actor3, descriptor + 8);
    } else if (field<u8>(config, 0x2c) == 1 && actor4) {
        actorTime = call_14231f230(actor4, descriptor + 8);
    }
    ptr world = virtual_ptr(self, 0x150);
    float now = field<float>(world, 0x540);
    int random = native_rand();
    float fraction = static_cast<float>(random) * 3.051851e-05f;
    if ((fraction < field<float>(config, 0x18) || fraction == field<float>(config, 0x18)) &&
        (!previous || field<float>(config, 0x1c) <= now - field<float>(previous, 0)) &&
        (actorTime == -1.0f || field<float>(config, 0x20) <= now - actorTime)) {
        if (magnitude <= 0.0f) magnitude = field<float>(config, 0x30);
        DispatchPayload payload;
        payload.actor3 = actor3;
        payload.actor4 = actor4;
        payload.kind = field<u64>(descriptor, 0);
        payload.key1 = field<u64>(descriptor, 8);
        payload.key2 = field<u64>(descriptor, 16);
        payload.magnitude = magnitude;
        payload.flag = flag;
        call_1424b6f30(self, &payload, dispatchFlag);
    }
    if (field<u8>(self, 0xf0) == 3 && actor4) {
        i32 second = field<i32>(descriptor, 12);
        i32 first = field<i32>(descriptor, 8);
        if ((first == global_i32(0x148161cc0) && second == global_i32(0x148161cc4)) ||
            (first == global_i32(0x148161cc8) && second == global_i32(0x148161ccc)) ||
            (first == global_i32(0x148161cd0) && second == global_i32(0x148161cd4)) ||
            (first == global_i32(0x148161cd8) && second == global_i32(0x148161cdc)) ||
            (first == global_i32(0x148161ce0) && second == global_i32(0x148161ce4)))
            call_1423524a0(actor4);
    }
}

// Native temporary spans four pointers: vtable, unknown, shared control, allocation.
struct VolumeTemporary { ptr vtable, unknown, control, allocation; };
u8 CanPlayerCreateInCurrentVolume(ptr self, ptr outVolume) {
    ptr manager = call_141c74820();
    ptr volume = manager ? call_141c7d420(manager, field<ptr>(self, 0x220)) : 0;
    field<ptr>(outVolume, 0) = volume;
    if (volume) {
        ptr world = virtual_ptr(self, 0x150);
        if (world) {
            ptr state = field<ptr>(world, 0x130);
            if (state) {
                ptr klass = AFortGameStateAthena_StaticClass();
                if (field<i32>(klass, 0x38) <= field<i32>(field<ptr>(state, 0x10), 0x38) &&
                    field<ptr>(field<ptr>(field<ptr>(state, 0x10), 0x30),
                               std::intptr_t(field<i32>(klass, 0x38)) * 8) == klass + 0x30 &&
                    call_14199e2c0(state)) return 1;
            }
        }
        volume = field<ptr>(outVolume, 0);
        ptr argument = call_14297b320();
        ptr component = virtual_ptr_arg(volume, 0x610, argument);
        if (component) {
            VolumeTemporary temporary; // constructed by call_14237f3d0
            ptr argument2 = call_14237f3d0(self, &temporary);
            u8 result = call_141eeab20(component, argument2);
            temporary.vtable = global_ptr_145a95888();
            if (temporary.allocation) FMemory_Free(temporary.allocation);
            if (temporary.control) {
                ptr control = temporary.control;
                --field<i32>(control, 8);
                if (field<i32>(control, 8) == 0) {
                    virtual_void(control, 0);
                    --field<i32>(control, 12);
                    if (field<i32>(control, 12) == 0) virtual_void_int(control, 8, 1);
                }
            }
            return result;
        }
    }
    return 1;
}
} // namespace feedback_volume75
