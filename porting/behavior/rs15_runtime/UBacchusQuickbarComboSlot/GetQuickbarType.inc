// Original body address: 0x144c235a0 (thunk 0x1451cefd0, group B, 15 instructions).
// Offset evidence: this+0x48c is the reflected SlotType (Enum, one byte). The asm maps the byte as 0->0, 1->1, 3->2, and 2 or >=4 -> 3.
// CONFIDENCE: HIGH
switch (static_cast<uint8>(SlotType))
{
case 0:
	return static_cast<EFortQuickBars>(0);
case 1:
	return static_cast<EFortQuickBars>(1);
case 3:
	return static_cast<EFortQuickBars>(2);
default:
	return static_cast<EFortQuickBars>(3);
}
