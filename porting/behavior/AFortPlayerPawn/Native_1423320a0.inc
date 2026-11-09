// CONFIDENCE: medium
// Original address: 0x1423320a0
// Vtable slot 0x998 fills a 32-byte out buffer; result byte compared to 3
uint8 outBuf[32];
char* pcVar1 = (char*)(*(void* (**)(void*, void*))(*(uint8**)self + 0x998))(self, outBuf);
return *pcVar1 == 3;
