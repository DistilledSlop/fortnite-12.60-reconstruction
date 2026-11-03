// CONFIDENCE: high
// Computation is in exec wrapper 0x1429c2450.
if (static_cast<uint8>(static_cast<uint8>(CurrentState) - 1) < 5)
{
    UWorld* World = GetWorld();
    if (World && World->GetGameState())
    {
        const float Remaining = ServerTime.EndTime - World->GetGameState()->GetServerWorldTimeSeconds();
        return Remaining <= 0.0f ? 0.0f : Remaining;
    }
}
return 0.0f;
