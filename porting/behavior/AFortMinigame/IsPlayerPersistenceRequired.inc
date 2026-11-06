// CONFIDENCE: high; recovered native control flow, see native-evidence.json.
// Native 0x14209a9c0; wrapper 0x1429c3830.
if (!PlayerState) return false;
if (PlayerPersistence == EMinigamePlayerPersistence::PartyLeaderOnly)
{
    return PlayerState->GetUniqueId() == PlayerState->PartyOwnerUniqueId;
}
return PlayerPersistence == EMinigamePlayerPersistence::AllPlayers;
