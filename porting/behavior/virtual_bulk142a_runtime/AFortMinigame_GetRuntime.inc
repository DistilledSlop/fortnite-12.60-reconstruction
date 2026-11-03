// CONFIDENCE: high
// Computation is in exec wrapper 0x1429c1020.
if (static_cast<uint8>(static_cast<uint8>(CurrentState) - 1) < 5)
{
    UWorld* World = GetWorld();
    if (World && World->GetGameState())
    {
        const float Elapsed = World->GetGameState()->GetServerWorldTimeSeconds() - ServerTime.StartTime;
        return Elapsed <= 0.0f ? 0.0f : Elapsed;
    }
}
return 0.0f;
