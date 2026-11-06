// CONFIDENCE: HIGH
const AGameStateBase* State = World ? World->GetGameState() : nullptr;
    const AFortGameStateZone* Zone = Cast<AFortGameStateZone>(State);
    if (Zone && !Cast<AFortGameStateAthena>(State))
    {
        int32 NumberPlayers = -1;
        float Difficulty = -1.0f;
        FString ZoneName, MissionName;
        Zone->NativeGetLoadAnalyticsValues(NumberPlayers, Difficulty, ZoneName, MissionName);
        Attributes.Reserve(Attributes.Num() + 4);
        Attributes.Emplace(TEXT("NumberPlayers"), static_cast<double>(NumberPlayers));
        Attributes.Emplace(TEXT("ZoneName"), ZoneName);
        Attributes.Emplace(TEXT("GameDifficulty"), static_cast<double>(Difficulty));
        Attributes.Emplace(TEXT("MissionName"), MissionName);
    }
