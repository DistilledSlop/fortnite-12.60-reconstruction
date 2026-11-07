// Original body address: 0x1424e7ac0 (thunk 0x14292af80); tail JNZ 0x141d098a0 (leaf copy) inlined.
// Offset evidence: this+0xf0 = AActor::Role (Byte, raw: AActor not generated); this+0xfb1 = bEnablePickupManagement (reflected bool bitfield, mask 1); this+0xfb8 = NativeRecoveredPtr_0xfb8 (object pointer, pointee class unknown); pointee+0xd0 (16B) and +0xe0 (4B) = FPickupManagementSettings (0x14 bytes).
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
if (Raw[0xf0] != 3 || !bEnablePickupManagement)
{
    return;
}
const uint8* Manager = reinterpret_cast<const uint8*>(NativeRecoveredPtr_0xfb8);
if (Manager == nullptr)
{
    return;
}
PickupSettings = *reinterpret_cast<const FPickupManagementSettings*>(Manager + 0xd0);
