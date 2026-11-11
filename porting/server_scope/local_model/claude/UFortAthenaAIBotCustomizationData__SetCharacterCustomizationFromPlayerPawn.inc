// CONFIDENCE: MEDIUM. Copies the cosmetic loadout of the given player pawn into this bot customization state. A null pawn is ignored.
// Original addresses: exec 0x1427c6190; helper 0x1418802e0 (-> copy-assign 0x140a003b0 and loadout accessor 0x142323510).
if (!InFortPawn)
{
	return;
}
CustomizationState->CosmeticLoadout = InFortPawn->GetPawnCosmeticLoadout();   // accessor 0x142323510 returns pawn+0x1918 (warns if not initialised)
