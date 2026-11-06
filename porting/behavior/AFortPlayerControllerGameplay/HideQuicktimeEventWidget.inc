// CONFIDENCE: medium
// Original address: 0x142a65320
// FFrame::Code at Stack+0x20: skip the opcode byte (advance if non-null)
*(ulonglong*)((uint8*)Stack + 0x20) = (ulonglong)(*(longlong*)((uint8*)Stack + 0x20) != 0) + *(longlong*)((uint8*)Stack + 0x20);
// virtual dispatch on this: vptr + byte offset 0x1ff8
(*(void(**)(void*))((*(uint8**)this) + 0x1ff8))(this);
