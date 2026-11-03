// UNCOUNTED source lift. Native_* fields and dispatch bindings below are
// placeholders for recovered offsets/calls, not implementations. Do not compile.
// All methods preserve original Win64 evaluation and live reads.

// 0x1411a5ab0; same conceptual operations as UE 4.25 SetAvoidanceEnabled.
// NativeWorld resolves cached +0xa8, otherwise 0x1440b3260; +0x140 on the
// returned world is its avoidance manager. 0x143f80ad0 registers the component.
void Rough_SetAvoidanceEnabled(NativeMovement* self, bool enabled)
{
    if (NativeFlag(self, 0x140, 0x04) != enabled) {
        NativeInt32(self, 0x1e4) = 0;
        NativeWorld* world = NativePointer<NativeWorld>(self, 0xa8);
        NativeSetFlag(self, 0x140, 0x04, enabled);
        if (!world)
            world = Native_1440b3260(self);
        NativeAvoidanceManager* manager = NativePointer<NativeAvoidanceManager>(world, 0x140);
        if (manager && enabled)
            Native_143f80ad0(manager, self, NativeFloat(self, 0x1f4));
    }
}

// 0x141918950: array storage +0x798, live count +0x7a0, stride 0x160.
// Component element and +0xb8 float remain unbound; invalid index returns 1.
float Rough_GetSpringLengthRatio(NativeVehicle* self, int32 index)
{
    if (index < 0 || NativeInt32(self, 0x7a0) <= index)
        return 1.0f;
    const char* data = NativePointer<char>(self, 0x798);
    return NativeFloat(data + int64(index) * 0x160, 0xb8);
}

// 0x1419153c0: do not fold the global gate; it can change independently.
int32 Rough_GetCurrentBudgetBits(NativeVehicle* self)
{
    if (NativeGlobalInt32(0x1478c24d8) == 0)
        return 0;
    return NativeInt32(self, 0x450);
}

// 0x14191bbc0: no null check in original; object and byte types unresolved.
uint8 Rough_IsCameraControlledRotationByte(NativeVehicle* self)
{
    const char* camera = NativePointer<char>(self, 0x738);
    return NativeByte(camera, 0x335);
}

// 0x1418f12e0: virtual call on the actor must retain override dispatch;
// the returned object's byte and bit are unresolved.
bool Rough_IsJumping(NativeVehicle* self)
{
    const char* state = NativeVehicleVirtual_b08(self);
    return (NativeByte(state, 0x28) & 0x02) != 0;
}

// 0x141920cf0 tail-calls 0x14190cca0 with self; both mutation and call matter.
void Rough_SetUnoccupiedAutoDestroy(NativeVehicle* self, float value)
{
    NativeFloat(self, 0xa3c) = value;
    Native_14190cca0(self);
}

// 0x141915870: raw 32-bit subtraction wraps; field identities unresolved.
uint32 Rough_GetCurrentGearBits(NativeVehicle* self)
{
    return NativeUInt32(self, 0x578) - NativeUInt32(self, 0x7e8);
}
