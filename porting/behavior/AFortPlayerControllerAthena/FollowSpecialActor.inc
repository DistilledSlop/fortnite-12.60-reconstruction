// CONFIDENCE: medium
// Original address: 0x142a58600
// FFrame Code advance as decompiled (Stack->Code, exec form)
*(ulonglong *)Stack->Code = (ulonglong)(*(longlong *)Stack->Code != 0) + *(longlong *)Stack->Code;
// Context->vftable: first qword of this, dereferenced as decompiled; slot +0x21e8
(*(void(**)(void*, int))((uint8*)(*(longlong*)(*(uint8**)this)) + 0x21e8))(this, 2);
