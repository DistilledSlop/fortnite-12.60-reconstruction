// CONFIDENCE: medium
// Original address: 0x142a1b6a0
// FFrame Code advance as decompiled (Stack->Code, exec form)
*(ulonglong *)Stack->Code = (ulonglong)(*(longlong *)Stack->Code != 0) + *(longlong *)Stack->Code;
// Context->vftable: first qword of this, dereferenced as decompiled; slot +0xbc0
uint8 uVar1 = (*(uint8(**)(void*))((uint8*)(*(longlong*)(*(uint8**)this)) + 0xbc0))(this);
*(uint8*)Result = uVar1;
