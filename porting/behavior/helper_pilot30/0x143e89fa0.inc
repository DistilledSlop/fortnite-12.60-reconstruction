// CONFIDENCE: high
// Original address: 0x143e89fa0
// raw byte offsets on param_1 (72-byte struct, layout unnamed), all zeroed
*(int32*)((uint8*)param_1 + 0x0) = 0;
*(int64*)((uint8*)param_1 + 0x8) = 0;
*(int64*)((uint8*)param_1 + 0x10) = 0;
*(int64*)((uint8*)param_1 + 0x18) = 0;
*(int64*)((uint8*)param_1 + 0x20) = 0;
*(int64*)((uint8*)param_1 + 0x28) = 0;
*(int64*)((uint8*)param_1 + 0x30) = 0;
*(int64*)((uint8*)param_1 + 0x38) = 0;
*(int64*)((uint8*)param_1 + 0x40) = 0;
return param_1;
