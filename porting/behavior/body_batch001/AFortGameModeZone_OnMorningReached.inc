// CONFIDENCE: medium
// Original address: 0x1428f3560
// FFrame::Code at Stack+0x20: skip the opcode byte (advance if non-null)
*(ulonglong*)((uint8*)Stack + 0x20) = (ulonglong)(*(longlong*)((uint8*)Stack + 0x20) != 0) + *(longlong*)((uint8*)Stack + 0x20);
// virtual dispatch on this: vptr + byte offset 0xc08
(*(void(**)(void*))((*(uint8**)this) + 0xc08))(this);
