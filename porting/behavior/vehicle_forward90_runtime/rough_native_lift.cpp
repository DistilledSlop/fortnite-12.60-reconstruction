// ROUGH / UNCOUNTED: source-level lift of native90 vehicle/deps probes.
// Not a manifest binding. Names beginning Native are unresolved UE types or
// dispatch bindings, NOT implemented helpers. Do not compile as-is. Windows
// interface this == AFortAthenaVehicle + 0x3b0; preserve all virtual dispatch.
// Type checks below denote UE class ancestry, not C++ RTTI.

struct NativeSeatEntry;       // stride 0xe8; occupant +0xa8, fallback +0xb0
struct NativeSeatArray;       // TArray layout: data +0, mutable count +8
struct NativeVehicleInterface;
struct NativeActor;
struct NativePawn;
struct NativeController;
struct NativeRefController;   // counters +8/+c, release virtuals +0/+8
struct NativePayload;         // 24-byte polymorphic value; vptr 0x1459aede0
struct NativeTwoArrays;       // two zeroed 16-byte arrays

// 0x141911a70, the concrete DestroyVehicle implementation. Actor virtual
// 0x778, interface virtual 0x670 (float result in XMM0), and actor virtual
// 0xc80 must retain their actual signatures and overridden dispatch.
void Rough_DestroyVehicle(NativeVehicleInterface* interfaceThis)
{
    NativeActor* actor = ActorAtInterfaceMinus3b0(interfaceThis);
    if (ActorByte(actor, 0xf0) != 3)
        return;

    ActorVirtual_778(actor);
    ActorByte(actor, 0x40d) |= 8;
    if ((ActorByte(actor, 0x40d) & 8) != 0) {
        NativePayload payload = NativePayloadWithVptrAndZeroWords(0x1459aede0);
        NativeTwoArrays arrays = ZeroNativeTwoArrays();
        const float value = InterfaceVirtual_670(interfaceThis);
        ActorVirtual_c80(actor, value, &arrays, 0, &payload, 0, 0);
        NativeRefController* ref = PayloadReferenceController(payload);
        RestorePayloadVptr(payload, 0x1459aede0);
        if (ref && --RefStrongCount(ref) == 0) {
            RefVirtual_0(ref);
            if (--RefWeakCount(ref) == 0)
                RefVirtual_8(ref, 1);
        }
        if (Array2Data(arrays)) FMemory_Free(Array2Data(arrays));
        if (Array1Data(arrays)) FMemory_Free(Array1Data(arrays));
    }
}

// 0x141f96c30, concrete ResetTrick. vtable +0x3a0 provides actor;
// +0x260 provides a pointer to an array header. Class_142a62c80 and
// Native_1423e6050 require type/method bindings. The count and array data
// are reloaded each iteration, including after calls that can mutate them.
void Rough_ResetTrick(NativeVehicleInterface* interfaceThis)
{
    NativeActor* actor = InterfaceVirtual_3a0(interfaceThis);
    if (ActorByte(actor, 0xf0) == 3)
        return;
    NativeSeatArray* seats = InterfaceVirtual_260(interfaceThis);
    for (int index = 0; index < SeatCount(seats); ++index) {
        NativePawn* occupant = SeatOccupant(SeatAt(seats, index));
        if (!occupant) continue;
        NativeController* controller = PawnControllerAt250(occupant);
        if (controller && IsA(controller, Class_142a62c80()))
            Native_1423e6050(controller, actor);
    }
}

// 0x141f860a0 / 0x141d04a20. +0x410 is virtual and returns a
// UFortVehicleSeatComponent-like object; concrete vehicle entry loads +0xaf0.
// The component's seats TArray lives at +0xc8. Reload its backing data after
// the IsA call, exactly as in the original, before reading a fallback.
void Rough_EjectAllPlayers(NativeVehicleInterface* interfaceThis)
{
    auto* component = InterfaceVirtual_410(interfaceThis);
    for (int index = 0; index < ComponentSeatCountAtD0(component); ++index) {
        NativeSeatEntry* entry = ComponentSeatAtC8(component, index);
        NativePawn* pawn = SeatOccupant(entry);
        if (!pawn) continue;
        NativeController* controller = PawnControllerAt250(pawn);
        if (controller && IsA(controller, Class_142a6ee20())) {
            ControllerVirtual_20f8(controller);
        } else {
            NativeController* fallback = SeatFallback(ComponentSeatAtC8(component, index));
            if (fallback) ControllerVirtual_20f8(fallback);
        }
    }
}

// 0x141f860c0 / 0x141d04ae0: return true only after dispatching an
// ejection, and continue searching after a matching seat with no controller.
bool Rough_EjectPlayerPawn(NativeVehicleInterface* interfaceThis, NativePawn* target)
{
    auto* component = InterfaceVirtual_410(interfaceThis);
    for (int index = 0; index < ComponentSeatCountAtD0(component); ++index) {
        NativeSeatEntry* entry = ComponentSeatAtC8(component, index);
        NativePawn* pawn = SeatOccupant(entry);
        if (target != pawn || !pawn) continue;
        NativeController* controller = PawnControllerAt250(pawn);
        if (controller && IsA(controller, Class_142a6ee20())) {
            ControllerVirtual_20f8(controller);
            return true;
        }
        NativeController* fallback = SeatFallback(ComponentSeatAtC8(component, index));
        if (fallback) {
            ControllerVirtual_20f8(fallback);
            return true;
        }
    }
    return false;
}
