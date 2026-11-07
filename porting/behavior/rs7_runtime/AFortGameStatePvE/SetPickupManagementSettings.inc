// Original body address: 0x1424f3850 (thunk 0x14292d410); tail JNZ 0x141d164b0 (leaf store) inlined.
// Offset evidence: this+0xf0 = AActor::Role (Byte, raw); this+0xfb1 = bEnablePickupManagement (reflected bool bitfield, mask 1); this+0xfb8 = NativeRecoveredPtr_0xfb8 (object pointer, pointee class unknown); pointee+0xd0 (16B), +0xe0 (4B) = FPickupManagementSettings stores; pointee+0xcc = byte set to 1.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
if (Raw[0xf0] != 3 || !bEnablePickupManagement)
{
    return;
}
uint8* Manager = reinterpret_cast<uint8*>(NativeRecoveredPtr_0xfb8);
if (Manager == nullptr)
{
    return;
}
*reinterpret_cast<FPickupManagementSettings*>(Manager + 0xd0) = PickupSettings;
Manager[0xcc] = 1;
