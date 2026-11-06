// Native 0x14209aa00. CONFIDENCE: high
if (!PlayerState) return false;
const FUniqueNetIdRepl& Id = PlayerState->GetUniqueId();
for (const FMinigamePlayer& Player : SpectatingPlayerArray.Players)
{
    const bool bPlayerValid = Player.UniquePlayerId.IsValid();
    const bool bIdValid = Id.IsValid();
    if (bPlayerValid == bIdValid && (!bPlayerValid || Player.UniquePlayerId == Id)) return true;
}
return false;
