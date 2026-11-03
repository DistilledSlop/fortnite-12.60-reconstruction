// CONFIDENCE: HIGH
NumberPlayers = 0;
    for (const APlayerState* Player : PlayerArray)
        if (Player && !Player->IsSpectator() && !Player->IsOnlyASpectator())
            ++NumberPlayers;
    Difficulty = GameDifficulty;
    const UFortZoneTheme* Theme = ZoneTheme;
    ZoneName = Theme ? Theme->GetName() : TEXT("<Unavailable>");
    MissionName = TEXT("NullMissionManager");
    const AFortMissionManager* Manager = MissionManager;
    if (Manager)
    {
        // NativeFindPrimaryMission is the separately ported 0x1420bc640.
        if (const AFortMission* Mission = Manager->NativeFindPrimaryMission())
            MissionName = Mission->NativeMissionText690().ToString();
    }
