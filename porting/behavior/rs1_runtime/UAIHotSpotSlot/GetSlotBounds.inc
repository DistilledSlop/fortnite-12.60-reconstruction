// Behaviour snippet: UAIHotSpotSlot::GetSlotBounds (body only).
// Original body address: 0x14165f070 (thunk 0x142710a10), receiver delta 0. The returned FBox is written through the hidden return pointer (RDX).
// Evidence: Height (float +0x60) and Radius (float +0x64) are reflected. The floats at +0xa0, +0xa4, +0xa8 are unreflected in the class's own region (lowest own offset 0x0): NativeRecoveredFloat_0xa0/0xa4/0xa8. Half height uses the constant 0.5 (decompile of 0x14165f070). Min = (x-R, y-R, z), Max = (x+R, y+R, z+2*half), IsValid = 1, with the same float operation order as the body.
// CONFIDENCE: HIGH
const float HalfHeight = Height * 0.5f;
const float ZTop = NativeRecoveredFloat_0xa8 + HalfHeight;
FBox Bounds;
Bounds.Min = FVector(NativeRecoveredFloat_0xa0 - Radius, NativeRecoveredFloat_0xa4 - Radius, ZTop - HalfHeight);
Bounds.Max = FVector(NativeRecoveredFloat_0xa0 + Radius, NativeRecoveredFloat_0xa4 + Radius, ZTop + HalfHeight);
Bounds.IsValid = 1;
return Bounds;
