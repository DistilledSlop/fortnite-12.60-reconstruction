// Original address: 0x1420ef2b0
// CONFIDENCE: high
if (AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
{
    const uint8 AxisBias = Player->NativeGetSkyTubeAxisBias1409a95b0();
    if (AxisBias == 3)
    {
        return true;
    }
    if (AxisBias != 0 && Player->CurrentSkyTube != nullptr)
    {
        const FVector WorldLoc = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
        float InputKey = 0.0f;
        FVector SplineLoc = FVector::ZeroVector;
        FVector SplineDir = FVector::ZeroVector;
        Player->CurrentSkyTube->NativeSampleSplineAtLocation142701530(WorldLoc, InputKey, SplineLoc, SplineDir);
        const bool bVerticalSpline = FMath::Abs(SplineDir.Z) > 0.9f;
        return bVerticalSpline != (AxisBias == 2);
    }
    return false;
}
return true;
