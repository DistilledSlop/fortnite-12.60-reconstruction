// CONFIDENCE: medium
// Original address: 0x14084be80
// Vtable slot 0xb08 of param_1 returns a pointer; the uint32 stored there is the result.
uint32* puVar1 = (uint32*)(*(void*(**)(void*))((*(uint8**)param_1) + 0xb08))(param_1);
return *puVar1;
