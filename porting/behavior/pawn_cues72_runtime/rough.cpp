// UNCOUNTED rough source; not a build input. All external contracts below remain
// unresolved. Native addresses identify effects, never replacement no-op bodies.
#include <cstdint>
#include <cstring>
#include <initializer_list>
namespace pawn_cues72 {
using Byte = std::uint8_t;
struct Pawn;
template<class T> T& At(void* p, unsigned offset) {
    return *reinterpret_cast<T*>(static_cast<Byte*>(p) + offset);
}
extern void* AbilitySystemComponent(Pawn*); // exact reflected member, not a cache
extern void* Globals_141ed1800();
extern void* GlobalAddress_1459aede0();
extern void Cue_1406ce540(void*, std::uint64_t, int, void*);
extern void Free(void*);
extern void ControlVirtual0(void*); // strong-zero destruction
extern void ControlVirtual8(void*, int); // weak-zero destruction
extern int Mode_143f7a920_UnresolvedReceiver();
extern void DamageCues_142345360(Pawn*, void*, void*);

void ReleaseStrong(void* control) {
    if (control && --At<std::int32_t>(control, 8) == 0) {
        ControlVirtual0(control);
        if (--At<std::int32_t>(control, 12) == 0)
            ControlVirtual8(control, 1);
    }
}

// exec 142a20ea0, body after P_FINISH. Original signed byte is gap8[51a].
void StopBatchedLoopingWeaponActivation(Pawn* self) {
    if (At<std::int8_t>(self, 0x522) < 0 || !AbilitySystemComponent(self)) return;
    void* globals = Globals_141ed1800();
    // Contiguous native stack locals c8..14. Layout is retained as opaque data:
    // context descriptor, shared control and four allocation-bearing arrays.
    alignas(8) Byte storage[0xb8]{};
    At<void*>(storage, 8) = GlobalAddress_1459aede0();
    At<std::uint64_t>(storage, 0x90) = 0xffffffffULL; // stack38
    At<std::uint64_t>(storage, 0x98) = 0xffffffffULL; // stack30
    At<std::uint32_t>(storage, 0x88) = 0xffffffffU;  // stack40
    At<std::uint64_t>(storage, 0xa0) = 0xffffffffULL; // stack28
    At<std::uint64_t>(storage, 0xa8) = 0x100000001ULL;
    At<std::uint32_t>(storage, 0xb0) = 0xffffffffU; // stack18
    Cue_1406ce540(AbilitySystemComponent(self),
                 At<std::uint64_t>(globals, 0x1518), 3, storage);
    for (unsigned offset : {0x60u, 0x50u, 0x40u, 0x30u}) {
        if (void* allocation = At<void*>(storage, offset)) Free(allocation);
    }
    void* control = At<void*>(storage, 0x18); // stackb0
    At<void*>(storage, 8) = GlobalAddress_1459aede0();
    ReleaseStrong(control);
}

// implementation 142334fc0. Decompiler's formal SharedData/NonSharedData names
// contradict the widths it reads: arg2 is 68 bytes, arg3 is 16 bytes. Preserve
// argument positions until native RPC/struct ABI is resolved. No reflected
// replacement struct is silently substituted for these raw layouts.
void NetMulticast_Athena_BatchedDamageCues_Implementation(
    Pawn* self, void* arg2_68, void* arg3_16) {
    if (Mode_143f7a920_UnresolvedReceiver() == 1) return;
    alignas(8) Byte local68[0x44];
    alignas(8) Byte local16[0x10];
    std::memcpy(local16, arg3_16, sizeof local16);
    std::memcpy(local68, arg2_68, sizeof local68);
    DamageCues_142345360(self, local68, local16);
    // Reread inputs after the first dispatch; helper may mutate aliased state.
    auto second = At<std::uint64_t>(arg3_16, 8);
    if (second && second != At<std::uint64_t>(arg3_16, 0)) {
        At<std::uint64_t>(arg3_16, 0) = second;
        std::memcpy(local16, arg3_16, sizeof local16);
        // Native writes high byte before low byte. These are observable input
        // mutations before making the second complete copy, not local edits.
        At<Byte>(arg2_68, 0x1e) = At<Byte>(arg2_68, 0x41);
        At<Byte>(arg2_68, 0x1d) = At<Byte>(arg2_68, 0x40);
        std::memcpy(local68, arg2_68, sizeof local68);
        DamageCues_142345360(self, local68, local16);
    }
}
} // namespace pawn_cues72
