// CONFIDENCE: HIGH
int32 Level = 0;
if (APawn* Pawn = Cast<APawn>(CachedContext))
{
    if (AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Pawn->GetPlayerState()))
    {
        Level = PlayerState->KillScore;
        if (GameIsReverse.GetValueAtLevel(0.0f, nullptr) > 0.0f)
        {
            Level = static_cast<int32>(static_cast<uint32>(NativeReverseLevelOrigin) - static_cast<uint32>(Level));
        }
    }
}
OutInfiniteAmmo = UseInfiniteAmmo.GetValueAtLevel(static_cast<float>(Level), nullptr) > 0.0f;
return 2;
