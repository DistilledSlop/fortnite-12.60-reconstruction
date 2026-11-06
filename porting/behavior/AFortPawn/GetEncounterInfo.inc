// CONFIDENCE: medium
// Original address: 0x142a1c1a0
// FFrame Code advance as decompiled (Stack->Code, exec form)
*(ulonglong *)Stack->Code = (ulonglong)(*(longlong *)Stack->Code != 0) + *(longlong *)Stack->Code;
// Context->_gap_0x8 + 0x4e0 = this+0x4e8 (raw); call through the pointer stored there at slot +0x10, no receiver as decompiled
uint64 uVar1 = (*(uint64(**)(void))((uint8*)(*(longlong*)((uint8*)this + 0x4e8)) + 0x10))();
*(uint64*)Result = uVar1;
