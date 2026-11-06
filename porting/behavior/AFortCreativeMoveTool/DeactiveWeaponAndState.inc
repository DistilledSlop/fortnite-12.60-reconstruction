// CONFIDENCE: medium
// Original address: 0x1428c2420
// FFrame::Code at Stack+0x20: skip the opcode byte (advance if non-null)
*(ulonglong*)((uint8*)Stack + 0x20) = (ulonglong)(*(longlong*)((uint8*)Stack + 0x20) != 0) + *(longlong*)((uint8*)Stack + 0x20);
// virtual dispatch on this: vptr + byte offset 0xc60
(*(void(**)(void*))((*(uint8**)this) + 0xc60))(this);
