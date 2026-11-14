    // CONFIDENCE: HIGH
    // 0x141e628f0: the original assumes the game instance has the Fortnite type.
    UGameInstance* Instance = GetGameInstance();
    return Instance ? static_cast<UFortGameInstance*>(Instance)->PlaylistManager : nullptr;
