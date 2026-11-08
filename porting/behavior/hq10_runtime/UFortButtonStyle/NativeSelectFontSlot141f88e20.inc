// Helper 0x141f88e20 (UFortButtonStyle), callers 0x144d79e90 and 0x144d79440 (RCX = embedded struct at +0x17b8 / style, EDX = index).
// Asm evidence: index 0 -> RCX; 1 -> +0x50; 2 -> +0xa0; 4 -> +0x140; 5 -> +0x190; otherwise (3, <0, >=6) -> +0xf0. Returns the address (LEA/MOV).
// CONFIDENCE: HIGH
switch (SlotIndex)
{
case 0:
	return Style;
case 1:
	return Style + 0x50;
case 2:
	return Style + 0xa0;
case 4:
	return Style + 0x140;
case 5:
	return Style + 0x190;
default:
	return Style + 0xf0;
}
