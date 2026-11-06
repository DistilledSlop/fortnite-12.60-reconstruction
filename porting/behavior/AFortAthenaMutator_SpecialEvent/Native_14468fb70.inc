// CONFIDENCE: medium
// Original address: 0x14468fb70
// Reason: Raw byte offsets 0x8 on param_2 and 0x1c8 on param_1; struct layout unnamed.
// raw byte offset 0x8 on param_2 (int32 field)
*(int32*)((uint8*)param_2 + 0x8) = 0;
// raw byte offset 0x1c8 on param_1 (address of embedded field)
*param_2 = param_1 + 0x1c8;
return param_2;
