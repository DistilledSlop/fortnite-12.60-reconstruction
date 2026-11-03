// CONFIDENCE: HIGH
const AFortGameStateAthena* State = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
    if (State)
    {
        // The original obtains both values before appending any attributes.
        const float MatchTime = State->NativeMatchTime780();
        const uint8 Phase = static_cast<uint8>(State->GamePhase);
        const FString MapName = World->GetMapName();
        Attributes.Reserve(Attributes.Num() + 3);
        Attributes.Emplace(TEXT("AthenaGamePhase"), AFortGameStateAthena::NativeGetAthenaPhaseName(Phase));
        Attributes.Emplace(TEXT("AthenaMatchTimeSec"), static_cast<double>(MatchTime));
        Attributes.Emplace(TEXT("AthenaMapName"), MapName);
    }
    return Attributes;
