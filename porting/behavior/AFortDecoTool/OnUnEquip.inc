// CONFIDENCE: medium
// Original address: 0x14283dc50
// FFrame::Code at Stack+0x20: skip the opcode byte (advance if non-null)
*(ulonglong*)((uint8*)Stack + 0x20) = (ulonglong)(*(longlong*)((uint8*)Stack + 0x20) != 0) + *(longlong*)((uint8*)Stack + 0x20);
// virtual dispatch on this: vptr + byte offset 0x778
(*(void(**)(void*))((*(uint8**)this) + 0x778))(this);
