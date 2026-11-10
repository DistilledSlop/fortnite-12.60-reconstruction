// dep AFortGameStateZone__ResolveMissionRewardLootLevelOverride original 0x143f7a920
// CONFIDENCE: LOW. Original 0x143f7a920 (also reached through thunk 0x141eb0560). Only the object is passed; no loot level is consumed.
UObject* Owner = GetOwnerForLootLevel();   // vtable +0x150 on this
const FName Key = ResolveKeyForLootLevel;  // this+0xe8
UObject* Resolved = nullptr;
if (Key == NAME_0x11a && Key.Number == 0)   // FName_EqualsEName(...,0x11a) and high dword 0
{
	if (Owner == nullptr)
	{
		return 0;
	}
	Resolved = Owner->LootLevelSource;   // +0x38
}
else
{
	Resolved = FindLootLevelRegistryEntry(Registry, Owner, Key);   // 0x14463f3c0 (engine registry lookup, not read in full)
}
if (Resolved == nullptr && (Owner == nullptr || (Resolved = Owner->LootLevelFallback) == nullptr))   // +0xf0
{
	return 0;
}
return ClassifyLootLevelSource(Resolved);   // 0x1443771a0
