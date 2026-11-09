// CONFIDENCE: medium
// Original address: 0x142a7d020
// Raw FFrame::Code advance (offset 0x20): Code += (Code != 0)
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
// Virtual dispatch through the object vtable (raw byte offset)
uint8 uVar1 = (*(uint8 (**)(void*))(*(uint8**)this + 0x1310))(this);
*(uint8*)Result = uVar1;
