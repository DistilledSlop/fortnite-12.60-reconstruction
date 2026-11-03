// UNCOUNTED behavioral drafts, not native build inputs. Externs require faithful
// ABI/effect recovery; no default or empty implementations are supplied.
#include <cstdint>
#include <cstring>
namespace pickup75 {
using Byte = std::uint8_t;
using Word = std::uint64_t;
struct Object;
template<class T> T& At(void* p, unsigned offset) {
    return *reinterpret_cast<T*>(static_cast<Byte*>(p) + offset);
}
extern bool Predicate_143f7abb0_UnresolvedArgs();
extern void Call_143f72040(Object*);
extern void Call_143f84be0(Object*, int);
extern void Virtual720(Object*, Word, Object*, std::uint32_t, Byte);
extern void* Class_142a3d9f0();
extern bool IsA_OriginalClassIndexTable(Object*, void*);
extern void Call_142038060(Object*);
extern std::uint32_t GlobalBits_14813b798(); // type remains unresolved
extern void Call_1423bd380(Object*, std::uint32_t);
extern void Virtual5e8(Object*);
extern void* Call_14203e6d0(void* scratch);
extern std::uint32_t Call_14202dc10(Object*);
extern void Virtual640_CapturedTable(void* table, Object*, Object*,
                                     std::uint32_t, const void*, Byte);

// native14203cda0; reflected wrapper142a2bad0 forwards these eight raw arguments.
void TossPickup(Object* self, Word arg2, Object* arg3, std::uint32_t arg4,
                Byte arg5, Byte arg6, Byte arg7, Byte arg8) {
    if (At<Byte>(self, 0xf0) != 3) return;
    if (!Predicate_143f7abb0_UnresolvedArgs()) Call_143f72040(self);
    else Call_143f84be0(self, 1);
    At<Byte>(self, 0x49c) = arg6;
    Virtual720(self, arg2, arg3, arg4, arg5);
    if (arg3) {
        Object* related = At<Object*>(arg3, 0x250);
        if (related) {
            void* type = Class_142a3d9f0();
            if (IsA_OriginalClassIndexTable(related, type)) {
                Call_142038060(self);
                Call_1423bd380(related, GlobalBits_14813b798());
            }
        }
    }
    At<Byte>(self, 0x450) |= arg7;
    At<Byte>(self, 0x451) = arg8;
    Virtual5e8(self);
}

// native1420279f0. Raw12 preserves the observed 8-byte then4-byte copy;
// neither reflected argument labels nor the apparent helper establish a vector.
void BlueprintSetPickupTarget(Object* self, Object* arg2, Byte arg3) {
    if (!arg2) return;
    alignas(8) Byte scratch[16];
    void* value = Call_14203e6d0(scratch);
    void* table = At<void*>(self, 0); // captured before the next helper
    alignas(8) Byte local12[12];
    std::memcpy(local12, value, 8);
    std::memcpy(local12 + 8, static_cast<Byte*>(value) + 8, 4);
    std::uint32_t arg = Call_14202dc10(self);
    Virtual640_CapturedTable(table, self, arg2, arg, local12, arg3);
}
} // namespace pickup75
