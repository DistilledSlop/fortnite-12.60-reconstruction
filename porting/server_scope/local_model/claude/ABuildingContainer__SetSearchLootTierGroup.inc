// CONFIDENCE: MEDIUM. Chunk 01 port. See ABuildingContainer__SetSearchLootTierGroup.evidence.json for captures and notes.
// Original address: 0x14273c3f0 (exec). Body 0x141beb120.
// Store the tier group, then re-roll the search-loot random upgrade when this container is authoritative, in the right
// phase, and a roll is still pending.
SearchLootTierGroup = InSearchLootTierGroup;
if (ShouldRollRandomUpgrade())
{
	if (IsMatchingGamePhase() && IsRandomUpgradePending())
	{
		RollRandomUpgrade(true);
		return;
	}
	// A roll that was already resolved stays resolved; the decompile's flag re-set here is a no-op.
}
