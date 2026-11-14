// CONFIDENCE: HIGH. Original14236e2a0; virtual player-pawn/action dispatch retained.
AFortPlayerPawn* Pawn = GetPlayerPawn();
if (!Pawn) return false;
const uint8 Style = static_cast<uint8>(Pawn->CurrentMovementStyle.GetValue());
if (Style == 0)
{
    return Pawn->NativeCanPerformAction14230c260(ActionTag);
}
if (static_cast<uint8>(Style - 4) < 2)
{
    UFortGameData* GameData = UFortAssetManager::NativeGetFortAssetManager().NativeGetCommonGameData();
    UFortGameplayTagData* TagData = GameData->NativeResolveGameplayTagData1415e2820(true);
    if (TagData && ActionTag.MatchesTag(TagData->NativeActionEmote))
    {
        return Pawn->NativeCanPerformAction14230c260(ActionTag);
    }
}
return false;
