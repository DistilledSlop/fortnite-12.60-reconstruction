// Native 0x14209a460. CONFIDENCE: high
if (!UniqueNetId.IsValid()) return false;
for (const FMinigamePlayer& Player : PlayerArray.Players)
    if (Player.UniquePlayerId.IsValid() && Player.UniquePlayerId == UniqueNetId) return true;
if (bCheckSpectatingPlayers)
    for (const FMinigamePlayer& Player : SpectatingPlayerArray.Players)
        if (Player.UniquePlayerId.IsValid() && Player.UniquePlayerId == UniqueNetId) return true;
return false;
