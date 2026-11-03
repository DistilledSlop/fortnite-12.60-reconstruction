// UNCOUNTED behavioral recovery. Not a native build input; external operations
// and names below are provisional and must be bound to the original ABI.
#include <cstdint>
#include <cstring>
#include <cmath>
namespace pickup_rep92 {
struct Pickup; struct ItemEntry;
struct Vec3 { float x, y, z; };
template<class T> T& field(Pickup* p, unsigned off) {
    return *reinterpret_cast<T*>(reinterpret_cast<unsigned char*>(p) + off);
}
extern int NativeRand(); // 0x142033b40 calls rand; expected MSVC RAND_MAX=32767
extern const Vec3 Global147c31cc0, Global148218630;
struct MovementComponent; struct World;
extern bool ActorPredicate143f7abb0(Pickup*);
extern void ActorAction143f72040(Pickup*);
extern void ActorAction143f84be0(Pickup*, bool);
extern void ActorAction143f85010(Pickup*, bool);
extern void ActorAction143f84060(Pickup*, bool);
extern void MovementVirtual440(MovementComponent*, void*); // component virtual +440
extern std::uint32_t Value141ef6bc0(Pickup*);
extern void PickupVirtual700(Pickup*);
extern void PickupVirtual708(Pickup*);
extern World* PickupWorldVirtual150(Pickup*);
extern void PickupVirtual5e8(Pickup*);
template<class T, class P> T& raw(P* p, unsigned off) {
    return *reinterpret_cast<T*>(reinterpret_cast<unsigned char*>(p) + off);
}
// Call-site specialization of 0x14203cea0: the last three arguments are zero.
void ApplyImpulse14203cea0(Pickup* self, const Vec3& velocity) {
    if (field<std::uint8_t>(self, 0xf0) != 3) return;
    if (!ActorPredicate143f7abb0(self)) ActorAction143f72040(self);
    else ActorAction143f84be0(self, true);
    ActorAction143f85010(self, true);
    auto* movement = field<MovementComponent*>(self, 0x480);
    field<float>(self, 0x110) = 3.0f;
    if (movement) {
        raw<Vec3>(movement, 0xc4) = velocity;
        raw<float>(movement, 0xf4) = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
        MovementVirtual440(movement, field<void*>(self, 0x130));
        field<std::uint8_t>(self, 0x49d) = 0;
        field<std::uint32_t>(self, 0x5c8) = Value141ef6bc0(self);
    }
    field<std::uint8_t>(self, 0x450) |= 0x10;
    field<std::uint8_t>(self, 0x49b) = 0;
    field<std::uint8_t>(self, 0x49a) = 1;
    field<std::uint8_t>(self, 0x49c) = 0;
    PickupVirtual700(self);
    PickupVirtual708(self);
    ActorAction143f84be0(self, true);
    ActorAction143f84060(self, true);
    World* world = PickupWorldVirtual150(self);
    if (world) field<std::uint32_t>(self, 0x54c) = raw<std::uint32_t>(world, 0x540);
    PickupVirtual5e8(self);
}
// Reproduce native rsqrtss plus two Newton refinements, including float rounding.
extern float NativeRefinedReciprocalSqrt(float);

// HandleConnectedActorDestroyed, wrapper 0x142a2aee0 -> 0x142033b40.
void HandleConnectedActorDestroyed(Pickup* self) {
    if (field<std::uint8_t>(self, 0xf0) != 3 || !field<void*>(self, 0x480)) return;
    const float magnitude = float(NativeRand()) * 0.0045777764f + 350.0f;
    const Vec3 velocity { Global147c31cc0.x * magnitude,
                          Global147c31cc0.y * magnitude,
                          Global147c31cc0.z * magnitude };
    ApplyImpulse14203cea0(self, velocity);
    const float squared = velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z;
    Vec3 direction;
    if (squared == 1.0f) direction = velocity;
    else if (squared >= 1.e-8f) {
        const float r = NativeRefinedReciprocalSqrt(squared);
        direction = {r * velocity.x, r * velocity.y, r * velocity.z};
    } else direction = Global148218630;
    field<Vec3>(self, 0x424) = direction;
}

// 0x142a2ac00 and 0x142a2ac30 return literal IEEE-754 values.
float GetMovementCapsuleHalfHeight(const Pickup*) { return 60.0f; }
float GetMovementCapsuleRadius(const Pickup*) { return 30.0f; }

// 0x142a2ab30: the wrapper extracts an int32 index and copies the
// referenced entry into its result. Both helpers retain their native effects.
// Performs the thread-safe initialization check even for valid indices.
extern const ItemEntry* InitializedEmptyEntry14813c380();
const ItemEntry* EntryAt14202ecb0(Pickup* self, std::int32_t index) {
    const ItemEntry* empty = InitializedEmptyEntry14813c380();
    const auto count = field<std::int32_t>(self, 0x3e8);
    if (!count) return index == 0 ? reinterpret_cast<const ItemEntry*>(reinterpret_cast<unsigned char*>(self) + 0x290) : empty;
    if (index < 0 || index >= count) return empty;
    return reinterpret_cast<const ItemEntry*>(field<unsigned char*>(self, 0x3e0) + std::int64_t(index) * 0x150);
}
extern void CopyEntry142025400(ItemEntry* out, const ItemEntry* source);
void GetItemEntry(Pickup* self, std::int32_t index, ItemEntry* out) {
    CopyEntry142025400(out, EntryAt14202ecb0(self, index));
}
// 0x142a2abd0 -> 0x14202ed80.
std::int32_t GetItemEntryNum(Pickup* self) {
    const auto count = field<std::int32_t>(self, 0x3e8);
    return count != 0 ? count : 1;
}
} // namespace pickup_rep92
