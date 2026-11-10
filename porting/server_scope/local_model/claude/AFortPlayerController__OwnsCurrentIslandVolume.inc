// CONFIDENCE: LOW - the top-level decision is read; the island lookup (0x141c74820, 0x141c7c5c0, 180 lines), the level-save virtual (slot 0x610) and the identity virtuals (slots 0x28 and 0x0) are not ported.
// Original address: 0x142a49550 (exec thunk, bool return). Helper: 0x1423a0aa0. Island lookup uses the Pawn (AController::Pawn, offset 0x248).
// Return true when the local game account id and the level-save owner id agree in validity and value.
APawn* PawnRef = Pawn;
UObject* Island = FindIslandForPawn_0x141c74820(PawnRef);
if (Island == nullptr)
{
	return false;
}
UObject* IslandVolume = FindIslandVolumeForPawn_0x141c7c5c0(Island, PawnRef);
if (IslandVolume == nullptr)
{
	return false;
}
UClass* LevelSaveClass = FortLevelSaveComponent::StaticClass();
UObject* LevelSave = IslandVolume->FindComponentByClass_Slot0x610(LevelSaveClass);
if (LevelSave == nullptr)
{
	return false;
}
FUniqueNetIdRepl LocalAccountId = GetGameAccountId();
FUniqueNetIdRepl OwnerId = LevelSave->OwnerNetId_0x248;
const bool bLocalValid = LocalAccountId.IsValid();
const bool bOwnerValid = OwnerId.IsValid();
return bLocalValid == bOwnerValid && (!bLocalValid || LocalAccountId == OwnerId);
