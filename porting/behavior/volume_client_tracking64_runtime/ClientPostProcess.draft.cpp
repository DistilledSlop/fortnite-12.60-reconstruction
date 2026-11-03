// Source recovery for 0x141c93b20, NOT integrated or buildable.
// Offsets refer to shipping Win64 image; the remaining ABI bridges below need proof.
#include <cstdint>

struct UObject; struct UWorld; struct AActor; struct AFortPawn;
struct AController; struct UActorComponent; struct UPostProcessComponent;
struct AFortVolume;
struct FName { int32_t comparison_index, number; };
struct WeakObject { int32_t index, serial; };
struct SparseComponentElement { UActorComponent* component; uint64_t hash_links; };
// TSparseArray backing of AActor's component set at +0x198. The bit array is
// inline for up to four words, otherwise uses the pointer at bit-array +0x10.
struct SparseComponents {
    SparseComponentElement* elements; int32_t num_slots, max_slots;
    uint32_t inline_bits[4]; uint32_t* heap_bits;
    int32_t bit_count, bit_capacity;
};
extern bool IsA_FortPawn(const AActor*); // class getter 0x142a174f0
extern bool IsA_PostProcessComponent(const UActorComponent*); // 0x1447c6c10
extern UWorld* GetWorld_v150(const AActor*);
extern bool Pawn_vdb0(const AFortPawn*); // virtual predicate; semantics unknown
extern const FName ComponentTag_148122c40; // live process global, not a new FName
extern bool IsNoneComparisonIndex(int32_t); // FName_EqualsEName(index, NAME_None)
extern UObject* WeakGet(const WeakObject*); // 0x142ee5dd0
extern bool Controller_v6a8(AController*); // local-controller predicate
extern AActor* Controller_v698(AController*); // current camera view target, 0x1444803e0
extern UWorld* PawnWorld(AFortPawn*); // vtable +0x150

template<class T> static T& at(void* p, unsigned n) {
    return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(p) + n);
}

// 0x142321a70; 0x14468fb70 creates an iterator over world +0x1c8 at index 0.
static AController* FindLocalControllerViewingPawn(AFortPawn* pawn) {
    UWorld* world = PawnWorld(pawn);
    if (!world) return nullptr;
    for (int32_t index = 0; index >= 0 && index < at<int32_t>(world, 0x1d0); ++index) {
        auto* entries = at<WeakObject*>(world, 0x1c8);
        auto* c = reinterpret_cast<AController*>(WeakGet(entries + index));
        if (c && Controller_v6a8(c) && Controller_v698(c) == reinterpret_cast<AActor*>(pawn))
            return c;
    }
    return nullptr;
}
// 0x14468f240: first weak controller entry, not first valid entry.
static AController* FirstController(UWorld* world) {
    if (at<int32_t>(world, 0x1d0) < 1) return nullptr;
    auto* entries = at<WeakObject*>(world, 0x1c8);
    return reinterpret_cast<AController*>(WeakGet(entries));
}
// 0x1440a6ab0: component tag exact FName equality, NAME_None always false.
static bool ComponentHasTag(UActorComponent* component, FName name) {
    if (IsNoneComparisonIndex(name.comparison_index) && name.number == 0) return false;
    auto* tags = at<FName*>(component, 0x60);
    int32_t count = at<int32_t>(component, 0x68);
    for (int32_t i = 0; i < count; ++i)
        if (tags[i].comparison_index == name.comparison_index && tags[i].number == name.number)
            return true;
    return false;
}

void ClientPostProcess_141c93b20(AFortVolume* volume, AActor* actor, bool entering) {
    if (!actor || !IsA_FortPawn(actor)) return;
    auto* pawn = reinterpret_cast<AFortPawn*>(actor);
    UWorld* world = GetWorld_v150(reinterpret_cast<AActor*>(volume));
    if (world) {
        AController* pawn_controller = FindLocalControllerViewingPawn(pawn);
        AController* first_controller = FirstController(world);
        if ((!first_controller || first_controller != pawn_controller) && !Pawn_vdb0(pawn)) return;
    }
    // Shipping 0x1406801e0 is Stub_ReturnFalse; retain the tag predicate even
    // though this particular executable always selects untagged components.
    const bool choose_tagged = false;
    auto& set = at<SparseComponents>(volume, 0x198);
    const uint32_t* bits = set.heap_bits ? set.heap_bits : set.inline_bits;
    const int32_t limit = set.bit_count;
    for (int32_t slot = 0; slot < limit; ++slot) {
        if (!(bits[slot >> 5] & (uint32_t(1) << (slot & 31)))) continue;
        UActorComponent* component = set.elements[slot].component;
        if (!component || !IsA_PostProcessComponent(component)) continue;
        if (ComponentHasTag(component, ComponentTag_148122c40) == choose_tagged)
            at<float>(component, 0x738) = entering ? 1.0f : 0.0f;
    }
}
