// CONFIDENCE: high
// Computation is in exec wrapper 0x1429c2580.
if (CurrentState == EFortMinigameState::Warmup)
{
    UWorld* World = GetWorld();
    if (World && World->GetGameState())
    {
        const float Remaining = ServerTime.WarmupTime - World->GetGameState()->GetServerWorldTimeSeconds();
        return Remaining <= 0.0f ? 0.0f : Remaining;
    }
}
return 0.0f;
