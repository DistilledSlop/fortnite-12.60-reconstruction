// Helper 0x14205b290 (AFortPlayerController), callers 0x142381130 and 0x142382cc0 (quickbar slot lookups).
// Asm evidence: SlotKind 1 -> RCX+0x2b0, SlotKind 2 -> RCX+0x340, otherwise (0 and >=3) -> RCX+0x220. Returns the address (LEA).
// CONFIDENCE: HIGH
uint8* RawOwner_hq10 = reinterpret_cast<uint8*>(Owner);
if (SlotKind == 1)
{
	return RawOwner_hq10 + 0x2b0;
}
if (SlotKind == 2)
{
	return RawOwner_hq10 + 0x340;
}
return RawOwner_hq10 + 0x220;
