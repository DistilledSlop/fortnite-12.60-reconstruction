// CONFIDENCE: medium
// Original address: 0x141be2510
uint32 X = *(uint32*)((uint8*)param_1 + 0xb30);
return ((((X >> 9) & 0xffffff) << 8) | ((X >> 1) & 0xff)) & 0xffffff01; // CONCAT31 then mask
