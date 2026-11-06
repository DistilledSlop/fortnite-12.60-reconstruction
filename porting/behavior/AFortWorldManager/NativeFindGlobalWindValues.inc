// CONFIDENCE: HIGH. Original 0x141f87230; ordered float comparisons are intentional.
if (InitialGlobalWind.WindIntensity < 0.0f)
{
    FFortWindIntensityAndDirection DiscoveredWind;
    DiscoveredWind.WindIntensity = 0.1f;
    DiscoveredWind.WindHeading = 45.0f;
    const bool bFound = NativeFindGlobalWind(DiscoveredWind);
    OutIntensity = DiscoveredWind.WindIntensity;
    OutDirection = FVector(1.0f, 0.0f, 0.0f).RotateAngleAxis(
        DiscoveredWind.WindHeading, FVector(0.0f, 0.0f, 1.0f));
    if (bFound)
        return true;
    UE_LOG(LogFortWind, Warning, TEXT("AFortWorldManager::FindGlobalWindValues failed to FindGlobalWindValues when initializing global wind."));
}
OutIntensity = InitialGlobalWind.WindIntensity;
OutDirection = FVector(1.0f, 0.0f, 0.0f).RotateAngleAxis(
    InitialGlobalWind.WindHeading, FVector(0.0f, 0.0f, 1.0f));
return InitialGlobalWind.WindIntensity >= 0.0f;
