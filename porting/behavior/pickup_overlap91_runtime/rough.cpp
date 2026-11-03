// UNCOUNTED source recovery: typed behavioral draft, not a native build input.
// Named externs mark unresolved ABI/effects; they are declarations, not stubs.
#include <cstdint>
#include <cmath>
#include <cstring>
#include <vector>
namespace pickup_overlap91 {
struct Object; struct Pawn; struct Pickup; struct ItemDefinition; struct Inventory;
struct Guid { std::uint32_t words[4]; };
struct Interaction; struct ExistingItem;
template<class T> T& at(void* p, unsigned offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(p) + offset);
}
extern bool IsPickup(Object*); // class getter 142a29210
extern bool Global_1478de50c();
extern bool PawnVirtual6b0(Pawn*);
extern bool PawnVirtualce0_WithGlobalTag6f8(Pawn*); // 141ed1800 + 6f8
extern float ScalableFloatAtZero(void*); // FScalableFloat_GetValueAtLevel
extern float WorldTimeFor(Object*); // vtable 150, result +540
extern bool IsClass142a75f10(Object*);
extern bool IsClass14285d610(Object*);
extern Object* Virtual1088ThenVirtual8(Object*); // null intermediate permitted
extern ItemDefinition* PickupDefinition1420327f0(Pickup*);
extern Object* GetWorldContext141ef2890(Pawn*, int); // pawn virtual 150 is input
extern Object* ResolveMode14237e580(Object*);
extern bool ModePredicate141e26450(Object*);
extern bool ModePredicate141e26460(Object*);
extern bool Stub_ReturnFalse(); // original calls twice, independently
extern Object* WorldStateAt130(Pawn*); // pawn virtual 150, result +130
extern bool IsClass1429279a0(Object*);
extern bool ComponentHasTag568(Pawn*); // pawn+bb8 component, global 141ed1800 +568
extern bool ComponentHasTag570(Pawn*); // same component, global +570
extern std::uint8_t ItemCategory(ItemDefinition*); // virtual 298
extern bool ItemVirtual3a0(ItemDefinition*);
extern Inventory* ResolveInventoryFromPawnController(Pawn*); // pawn+250 class 142a3d9f0, +598 virtual 38
extern int InventoryUsedSlots(Inventory*); // +218 virtual 30
extern int InventoryCapacity(Inventory*); // +218 virtual 28
extern bool IsClass142b5be40(ItemDefinition*);
extern std::uint8_t ItemSlotCost14205af70(ItemDefinition*);
extern std::vector<Pickup*>& QueuedAutoPickups(Pawn*); // reflected pawn+1c60
extern std::vector<ExistingItem*> MatchingStackItems142006b20(Inventory*, ItemDefinition*);
extern int ItemMaxStack(ItemDefinition*); // virtual 390, arg 0
extern int PickupQuantity(Pickup*, ItemDefinition*); // virtual 660
extern int ExistingQuantity(ExistingItem*); // virtual 368
extern bool PickupFlag22b(Pickup*);
extern std::vector<Guid> InventoryGuidsForCategory142006ca0(Inventory*, unsigned);
extern ExistingItem* ResolveGuid(Inventory*, const Guid&);
extern bool SameItemIdentity(ItemDefinition*, ExistingItem*); // 268/3a8 -> 142c40540 -> 142bcf780
extern std::uint8_t DefinitionRank(ItemDefinition*); // virtual 2b0
extern std::uint8_t ExistingRank142033440(ExistingItem*);
extern void RequestSwap142a77320(Pawn*, Pickup*, const Guid&, float);
extern Interaction* MakeInteraction141ee0b10(Pawn*); // class 142a75f10 cast or null, flag 1
extern bool PickupInteractionCanInteract(Pickup*, Interaction*); // pickup+218 virtual 58
extern void PickupInteractionInteract(Pickup*, Interaction*); // pickup+218 virtual 60
extern bool LogVerbosityGreaterThan4_1481348f0();
// Formats pickup's object name (pickup+18) with 142ce9f90, emits category
// 1481348f4 at verbosity 5, then releases the temporary string.
extern void LogPickupName(Pickup*, const wchar_t* format);
extern void ScheduleQueueCallback2_5Seconds(Pawn*); // 140f95ae0/1446278d0, callback 14230dfa0

// 1420289a0: the original resolves each participant to class 14285d610,
// directly or through v1088/v8. A former owner outside class 142a75f10 is allowed.
static Object* identity(Object* p) {
    if (IsClass14285d610(p)) return p;
    Object* q = Virtual1088ThenVirtual8(p);
    return q && IsClass14285d610(q) ? q : nullptr;
}
bool MayRepickup(Pickup* pickup, Pawn* pawn) {
    float delay = ScalableFloatAtZero(reinterpret_cast<unsigned char*>(pawn) + 0x1c88);
    float droppedAt = at<float>(pickup, 0x55c);
    if (delay < WorldTimeFor(reinterpret_cast<Object*>(pickup)) - droppedAt) return true;
    Object* owner = at<Object*>(pickup, 0x4c8);
    if (owner == reinterpret_cast<Object*>(pawn)) return false;
    if (!owner || !IsClass142a75f10(owner)) return true;
    Object* previousIdentity = identity(owner);
    if (!previousIdentity) return true;
    return identity(reinterpret_cast<Object*>(pawn)) != previousIdentity;
}

// 142357450. The declared calls preserve unresolved game-side interactions,
// inventory and timer effects explicitly; this is not a compilable integration.
void TryToAutoPickupWeapon(Pawn* pawn, Pickup* pickup) {
    Object* context = GetWorldContext141ef2890(pawn, 1);
    if (!context) return;
    Object* mode = ResolveMode14237e580(context);
    if (Stub_ReturnFalse() && mode && !ModePredicate141e26450(mode)) return;
    if (!Stub_ReturnFalse() && (!mode || !ModePredicate141e26460(mode))) return;
    Object* worldState = WorldStateAt130(pawn);
    if (worldState && IsClass1429279a0(worldState) && at<unsigned char>(worldState, 0xfc0)) return;
    const float droppedAt = at<float>(pickup, 0x55c);
    if (droppedAt == 0.0f || WorldTimeFor(reinterpret_cast<Object*>(pawn)) - droppedAt < 0.3f) return;
    ItemDefinition* definition = PickupDefinition1420327f0(pickup);
    if (at<void*>(pawn, 0xbb8)) {
        bool tag568 = ComponentHasTag568(pawn);
        bool tag570 = ComponentHasTag570(pawn);
        if (tag568 || tag570) return;
    }
    if (!definition) return;
    std::uint8_t category = ItemCategory(definition);
    Interaction* interaction = MakeInteraction141ee0b10(pawn);
    if (LogVerbosityGreaterThan4_1481348f0())
        LogPickupName(pickup, L"TryToAutoPickupWeapon: '%s'");
    if (!((category >= 0x14 && category <= 0x16) || category == 6 || ItemVirtual3a0(definition))) return;
    Inventory* inventory = ResolveInventoryFromPawnController(pawn);
    if (!inventory) return;
    auto& queue = QueuedAutoPickups(pawn);
    // Stable compaction of null entries before the slot count.
    for (auto it = queue.begin(); it != queue.end();) {
        if (!*it) it = queue.erase(it); else ++it;
    }
    // Native also calls 14205af70 on the incoming definition once for each
    // queued element when it has class 142b5be40; its effects remain unresolved.
    for (Pickup* ignored : queue) {
        (void)ignored;
        ItemDefinition* d = PickupDefinition1420327f0(pickup);
        if (d && IsClass142b5be40(d)) (void)ItemSlotCost14205af70(d);
    }
    std::uint8_t cost = IsClass142b5be40(definition) ? ItemSlotCost14205af70(definition) : std::uint8_t{1};
    const std::int32_t used = InventoryUsedSlots(inventory);
    const std::uint32_t queueCount = static_cast<std::uint32_t>(queue.size());
    const std::int32_t capacity = InventoryCapacity(inventory);
    // The native additions wrap at 32 bits before the signed capacity comparison.
    std::uint32_t thresholdBits = std::uint32_t(used) + queueCount + std::uint32_t(cost);
    std::int32_t threshold;
    std::memcpy(&threshold, &thresholdBits, sizeof threshold);
    bool full = capacity < threshold;
    auto mergeOrSwap = [&]() {
        int maxStack = ItemMaxStack(definition);
        if (maxStack > 1) {
            for (ExistingItem* item : MatchingStackItems142006b20(inventory, definition)) {
                if (PickupQuantity(pickup, definition) + ExistingQuantity(item) <= maxStack) {
                    (void)PickupInteractionCanInteract(pickup, interaction);
                    PickupInteractionInteract(pickup, interaction);
                    return;
                }
            }
        }
        if (PickupFlag22b(pickup) && category == 0x16) {
            for (const Guid& id : InventoryGuidsForCategory142006ca0(inventory, 0x16)) {
                ExistingItem* item = ResolveGuid(inventory, id);
                if (item && SameItemIdentity(definition, item) && ExistingRank142033440(item) < DefinitionRank(definition)) {
                    RequestSwap142a77320(pawn, pickup, id, 1.0f);
                    break;
                }
            }
        }
    };
    if (full) {
        if (LogVerbosityGreaterThan4_1481348f0())
            LogPickupName(pickup, L"TryToAutoPickupWeapon queue full: '%s'");
        mergeOrSwap(); return;
    }
    // Additional existing definitions come from queue element +468; retain the
    // same identity/rank rejection for both inventory and queued definitions.
    extern ExistingItem* QueuedDefinitionAt468(Pickup*);
    // Native builds one snapshot before running any identity/rank comparisons.
    // It re-queries category here, then resolves all inventory GUIDs, appends
    // all queued definitions and scans the entire snapshot after a rejection.
    const std::uint8_t queryCategory = ItemCategory(definition);
    std::vector<ExistingItem*> snapshot;
    for (const Guid& id : InventoryGuidsForCategory142006ca0(inventory, queryCategory)) {
        ExistingItem* item = ResolveGuid(inventory, id);
        if (item) snapshot.push_back(item);
    }
    for (Pickup* queued : queue) snapshot.push_back(QueuedDefinitionAt468(queued));
    bool rejected = false;
    for (ExistingItem* item : snapshot) {
        if (item && SameItemIdentity(definition, item)) {
            const std::uint8_t incomingRank = DefinitionRank(definition);
            const std::uint8_t existingRank = ExistingRank142033440(item);
            if (existingRank >= incomingRank) rejected = true;
        }
    }
    if (rejected) { mergeOrSwap(); return; }
    if (!PickupInteractionCanInteract(pickup, interaction)) {
        if (LogVerbosityGreaterThan4_1481348f0())
            LogPickupName(pickup, L"TryToAutoPickupWeapon local interact failed: '%s'");
        return;
    }
    PickupInteractionInteract(pickup, interaction);
    for (Pickup* queued : queue) if (queued == pickup) return;
    queue.push_back(pickup);
    ScheduleQueueCallback2_5Seconds(pawn);
}

// 1423366f0: overlap component and other overlap arguments are unused.
void OnCapsuleBeginOverlap(Pawn* pawn, Object* otherActor) {
    if (!otherActor || !IsPickup(otherActor)) return;
    Pickup* pickup = reinterpret_cast<Pickup*>(otherActor);
    if (!Global_1478de50c() || !PawnVirtual6b0(pawn) || (at<unsigned char>(pawn, 0x53a) & 2)) return;
    void* state1578 = at<void*>(pawn, 0x1578);
    if (!state1578 && !(at<unsigned char>(pawn, 0x10e7) & 1) && !PawnVirtualce0_WithGlobalTag6f8(pawn)) return;
    if (!MayRepickup(pickup, pawn)) return;
    if (!state1578) {
        void* component = at<void*>(pawn, 0x280);
        if (!(0.0001f < std::fabs(at<float>(component, 0xc4)) ||
              0.0001f < std::fabs(at<float>(component, 0xc8)) ||
              0.0001f < std::fabs(at<float>(component, 0xcc)))) return;
    }
    TryToAutoPickupWeapon(pawn, pickup);
}
} // namespace pickup_overlap91
