// CONFIDENCE: high; reflected fields and complete native exec captured.
{
    CurrentAmmoCount = CurrentAmmoCount == MIN_int32 ? MAX_int32 : FMath::Max(CurrentAmmoCount - 1, 0);
}
