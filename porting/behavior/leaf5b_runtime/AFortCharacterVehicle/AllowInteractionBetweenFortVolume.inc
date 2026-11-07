// Thunk 0x142860f70: asm file not in porting/generated/native*; checked the Ghidra decompile
//   porting/generated/decomp/AFortCharacterVehicle/AllowInteractionBetweenFortVolume__142860f70.c: receiver this+0x1048, vtable slot 0xc0 (thunk_delta 0x1048).
// Body 0x141934370: MOVZX EAX,byte [RCX+0x20d8]; RET. Object offset = 0x1048 + 0x20d8 = 0x3120.
// 0x3120 is the declared bool bAllowInteractBetweenFortVolumes (AFortCharacterVehicle.h line 72; bool_bits.json offset 12576 mask 1).
// CONFIDENCE: MEDIUM
return bAllowInteractBetweenFortVolumes;
