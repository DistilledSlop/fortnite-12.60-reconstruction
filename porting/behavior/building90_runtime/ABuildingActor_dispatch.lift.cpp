// CONFIDENCE: HIGH for the dispatch sequence and argument order only.
// Evidence lift; not integration-ready C++. Slot names and implementations are
// unresolved. VirtualSlot calls mean a fresh vptr lookup at each dispatch and
// preserve override dispatch; they are notation, not helper implementations.
//
// exec 0x142737640 reads BuildingOwner, SpawningController, then
// bUsePlayerBuildAnimations and dispatches to slot +0x810. Reflection records
// these parameters in reverse property-chain order; the wrapper's order is the
// native ABI order. Pointer types below are inferred from reflected names.
void ABuildingActor::InitializeKismetSpawnedBuildingActor_Lift(
    AActor* BuildingOwner, AController* SpawningController,
    bool bUsePlayerBuildAnimations)
{
    VirtualSlot<0x810>(this, BuildingOwner, SpawningController,
                       bUsePlayerBuildAnimations);
}

// Native 0x141bc24b0, called by exec 0x142737e30. The vptr is reloaded
// between calls; the first virtual can mutate the object before the second.
void ABuildingActor::RefreshRemovedFoliageAroundBounds_Lift(
    bool bUseNonCollidingBounds)
{
    VirtualSlot<0x988>(this);
    VirtualSlot<0x980>(this, bUseNonCollidingBounds);
}

// Base binding of +0x810: 0x141baf090. The mode values are kept numeric;
// the meaning and signature of the two virtuals are not established. Assembly
// shows all six arguments to +0x880 on BOTH branches, despite decompilation.
void ABuildingActor::InitializeKismetSpawnedBuildingActor_BaseLift(
    AActor* BuildingOwner, AController* SpawningController,
    bool bUsePlayerBuildAnimations)
{
    const uint16 ControllerValue = SpawningController
        ? ControllerPlayerStateValue_1423881c0(SpawningController) : uint16(0xffff);
    const uint32 Mode = bUsePlayerBuildAnimations ? 1 : 4;
    VirtualSlot<0x880>(this, Mode, ControllerValue, BuildingOwner, nullptr, false);
    VirtualSlot<0x888>(this, Mode); // reload vptr after previous call
}

// Base binding +0x988: 0x141bc8570. Native_ names are unresolved routines;
// these declarations describe observed behavior, not implementations.
void ABuildingActor::FoliageSlot988_BaseLift()
{
    if (!bDestroyFoliageWhenPlaced || Native_143f7a920(this) == 1)
        return;
    auto* Manager = FindFoliageManager_141f88460(VirtualSlot<0x150>(this));
    if (Manager)
        RemoveFoliageEntry_141f9f2c0(Manager, this);
}

// Base binding +0x980: 0x141bc2c30. Component fields at +0x100 and
// +0x10c are the FVector origin and extent of its FBoxSphereBounds;
// byte +0x84 bit 0 is not yet named. VirtualSlot<0x4c8> returns an int.
void ABuildingActor::FoliageSlot980_BaseLift(bool bUseNonCollidingBounds)
{
    if (!bDestroyFoliageWhenPlaced || Native_143f7a920(this) == 1)
        return;
    auto* Manager = FindFoliageManager_141f88460(VirtualSlot<0x150>(this));
    if (!Manager)
        return;

    FBox Bounds(ForceInit); // initial IsValid = false
    if (OverridePrimitivesToExcludeFoliage.Num() < 1)
    {
        // Native returns a 28-byte FBox through an out parameter; the final
        // boolean argument is false. Exact source-level virtual name unresolved.
        Bounds = VirtualSlot<0x548, FBox>(this, bUseNonCollidingBounds, false);
    }
    else
    {
        for (UPrimitiveComponent* Component : OverridePrimitivesToExcludeFoliage)
        {
            if (!Component || !NativeByteBit(Component, 0x84, 0x01))
                continue;
            if (!bUseNonCollidingBounds && !VirtualSlot<0x4c8>(Component))
                continue;

            const FVector Origin = NativeVectorAt(Component, 0x100);
            const FVector Extent = NativeVectorAt(Component, 0x10c);
            const FVector Min = Origin - Extent;
            const FVector Max = Origin + Extent;
            if (!Bounds.IsValid)
            {
                Bounds.Min = Min;
                Bounds.Max = Max;
                Bounds.IsValid = true;
            }
            else
            {
                // Explicit comparisons match the observed conditional writes.
                if (Min.X < Bounds.Min.X) Bounds.Min.X = Min.X;
                if (Min.Y < Bounds.Min.Y) Bounds.Min.Y = Min.Y;
                if (Min.Z < Bounds.Min.Z) Bounds.Min.Z = Min.Z;
                if (Bounds.Max.X < Max.X) Bounds.Max.X = Max.X;
                if (Bounds.Max.Y < Max.Y) Bounds.Max.Y = Max.Y;
                if (Bounds.Max.Z < Max.Z) Bounds.Max.Z = Max.Z;
            }
        }
    }
    if (Bounds.IsValid)
        QueueFoliageBounds_141f967e0(Manager, &Bounds, this);
}

// Dependency evidence sketches. NativeClassIsA denotes the observed class-chain
// check, not a recovered class name. The precise weak-key and container APIs
// remain unresolved. These are not integration-ready helper implementations.
uint16 ControllerPlayerStateValue_1423881c0(AController* Controller)
{
    auto* State = NativePointerAt(Controller, 0x220);
    if (State && NativeClassIsA(State, NativeClass_142a85850()))
        return NativeUInt16At(State, 0x336);
    return 0xffff;
}

auto* FindFoliageManager_141f88460(UWorld* World)
{
    auto* Context = Native_144640410(Global_148351eb8, World, 1);
    auto* Object = Context ? NativePointerAt(Context, 0x130) : nullptr;
    if (Object && NativeClassIsA(Object, NativeClass_142908ec0()))
        return NativePointerAt(Object, 0x3d0);
    return nullptr;
}

void QueueFoliageBounds_141f967e0(void* Manager, const FBox* Box, ABuildingActor* Actor)
{
    if (Native_143f7a920(Manager) == 1 || !Box->IsValid)
        return;
    // Convert Actor to the 8-byte key, then append a 0x24-byte element
    // [key:8][box:28] to the array at Manager+0x218. The count is
    // incremented before capacity growth and the element copy.
    auto Key = Native_142ee3b30(Actor);
    int32 Index = NativeInt32At(Manager, 0x220)++;
    if (NativeInt32At(Manager, 0x224) < Index + 1)
        Native_140e5e650(NativeAddressAt(Manager, 0x218), Index);
    NativeWriteKeyAndBox(Manager, Index, Key, *Box);
}

void RemoveFoliageEntry_141f9f2c0(void* Manager, ABuildingActor* Actor)
{
    if (Native_143f7a920(Manager) != 1)
    {
        auto Key = Native_142ee3b30(Actor);
        Native_143af03b0(Key);
    }
}

// 0x143f7a920 examines its receiver: slot +0x150 supplies a world;
// receiver +0xe8 is compared as FName against EName 0x11a. Depending on
// that name it chooses world +0x38 or Native_14463f3c0(global, world,
// name), falling back to world +0xf0. If neither object is available it
// returns zero; otherwise it returns Native_1443771a0(selected object).
// Return value 1 suppresses foliage operations. This may be a net-mode
// query, but its identity is not proven.
