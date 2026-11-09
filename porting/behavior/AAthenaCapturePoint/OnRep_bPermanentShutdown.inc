// CONFIDENCE: medium
// Original address: 0x14271b000
*(ulonglong *)Stack->Code = (ulonglong)(*(longlong *)Stack->Code != 0) + *(longlong *)Stack->Code;
uint8* lVar1 = (uint8*)(*(void*(**)(void*))((*(uint8**)this) + 0x150))(this);
// this + 0x938 is param_1 + 0x127 of the decompile (longlong* arithmetic)
*(uint32*)((uint8*)this + 0x938) = *(uint32*)(lVar1 + 0x540);
