// CONFIDENCE: MEDIUM. Original exec 0x1427d8180 (NetMulticast thunk). Parameter: Direction (EAileronRollDirection).
// The thunk dispatches to the virtual implementation at vtable slot 0xeb8, resolved to 0x1418f25f0.
// That implementation begins the roll only when the aileron-roll state component at +0x3b0 is not locked (vtable slot 0x558).
if (!NativeRecoveredAileronRollState_0x3b0->NativeIsAileronRollLocked())
{
    BeginAileronRollInternal(Direction);
}
