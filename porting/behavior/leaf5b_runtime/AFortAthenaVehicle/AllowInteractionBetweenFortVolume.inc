// Thunk 0x14283a4f0: ADD RCX,0x378 (thunk_delta 0x378), virtual call [RAX+0xc0] on the sub-object (wrapper_delta 0).
// Body 0x14084b3d0: MOVZX EAX,byte [RCX+0xd0]; SHR AL,4; AND AL,1; RET. Object offset = 0x378 + 0xd0 = 0x448, bit 4 (mask 0x10).
// Header bitfield bAllowInteractBetweenFortVolumes_Runtime:1 is at 0x448 mask 0x10 (bool_bits.json, AFortAthenaVehicle.h line 188).
// CONFIDENCE: HIGH
return bAllowInteractBetweenFortVolumes_Runtime;
