// CONFIDENCE: medium
// Original address: 0x142b455e0
// Raw FFrame::Code advance (offset 0x20): Code += (Code != 0)
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
// Virtual dispatch through the object vtable (raw byte offset)
(*(void (**)(void*))(*(uint8**)this + 0x758))(this);
