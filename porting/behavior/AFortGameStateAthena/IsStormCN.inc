// CONFIDENCE: medium
// Original address: 0x142920160
// FFrame::Code at Stack+0x20: skip the opcode byte (advance if non-null)
*(ulonglong*)((uint8*)Stack + 0x20) = (ulonglong)(*(longlong*)((uint8*)Stack + 0x20) != 0) + *(longlong*)((uint8*)Stack + 0x20);
uint8 uVar1 = Native_14199e4e0();
// Result is the out-parameter for the bool-like return value
*(uint8*)Result = uVar1;
