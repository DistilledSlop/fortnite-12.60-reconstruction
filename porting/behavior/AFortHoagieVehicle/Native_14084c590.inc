// CONFIDENCE: medium
// Original address: 0x14084c590
// Vtable slot 0xb08 of param_1 returns a pointer; the uint32 at +0x14 of it is the result.
uint8* lVar1 = (uint8*)(*(void*(**)(void*))((*(uint8**)param_1) + 0xb08))(param_1);
return *(uint32*)(lVar1 + 0x14);
