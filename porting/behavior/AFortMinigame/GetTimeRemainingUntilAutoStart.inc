// CONFIDENCE: high
// Computation is in exec wrapper 0x1429c24f0.
if (CurrentState == EFortMinigameState::PreGame)
{
    UWorld* World = GetWorld();
    if (World && World->GetGameState())
    {
        const float Remaining = ServerTime.StartTime - World->GetGameState()->GetServerWorldTimeSeconds();
        return Remaining <= 0.0f ? 0.0f : Remaining;
    }
}
return 0.0f;
