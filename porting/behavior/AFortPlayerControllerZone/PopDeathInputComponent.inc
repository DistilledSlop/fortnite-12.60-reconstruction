// CONFIDENCE: medium
// Original address: 0x142a70730
// Raw FFrame::Code advance (offset 0x20): Code += (Code != 0)
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
(*(void (**)(void*, void*))(*(uint8**)this + 0xbe0))(this, *(void**)((uint8*)this + 0x27E0));
