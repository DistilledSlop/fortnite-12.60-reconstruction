// Original body address: 0x1450cbae0 (thunk 0x145258310)
// Offset evidence: dword [RCX+0xe4] vs SlotName.Number and dword [RCX+0xe0] vs SlotName.ComparisonIndex; equal -> return. Otherwise qword store to [RCX+0xe0] (NativeRecoveredUint64_0xe0), then CALL 0x1406a9dc0 delegate Broadcast on +0x90 (OnDifferentHeroLoadoutSlotFocused).
// CONFIDENCE: HIGH
const int32* Incoming = reinterpret_cast<const int32*>(&SlotName);
const int32* Stored = reinterpret_cast<const int32*>(&NativeRecoveredUint64_0xe0);
if (Stored[0] == Incoming[0] && Stored[1] == Incoming[1])
{
    return;
}
NativeRecoveredUint64_0xe0 = *reinterpret_cast<const uint64*>(&SlotName);
OnDifferentHeroLoadoutSlotFocused.Broadcast(SlotName);
