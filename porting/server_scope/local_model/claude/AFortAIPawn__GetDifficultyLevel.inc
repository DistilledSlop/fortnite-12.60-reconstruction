// CONFIDENCE: MEDIUM. Returns the pawn's difficulty level. AI type 5 returns the stored AIDifficultyLevel; other pawns read the level from the current game state when it is a FortGameStateZone, else 0.
// Original address: 0x1427a4910 (exec). Native body: 0x1422c8780 (uses GetWorld, FortGameStateZone class 0x142927a60, level accessor 0x141e9b630).
if (static_cast<uint8>(AIType) == 5)
{
    return AIDifficultyLevel;
}

UWorld* World = GetWorld();
if (!World)
{
    return 0.0f;
}

AGameStateBase* GameState = World->GetGameState();
if (GameState && GameState->IsA(AFortGameStateZone::StaticClass()))
{
    return Cast<AFortGameStateZone>(GameState)->DifficultyLevel;
}
return 0.0f;
