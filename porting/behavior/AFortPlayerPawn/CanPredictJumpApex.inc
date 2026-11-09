// CONFIDENCE: medium
// Original address: 0x142a7b0d0
// Raw FFrame::Code advance (offset 0x20): Code += (Code != 0)
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
uint8 uVar1 = Native_14230c280(this);
*(uint8*)Result = uVar1;
