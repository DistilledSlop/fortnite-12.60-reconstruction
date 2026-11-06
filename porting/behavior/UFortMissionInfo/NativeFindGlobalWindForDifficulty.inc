// CONFIDENCE: HIGH
// 0x1420bc010: selection is cached in the chosen asset entry.
const int32 Index = NativeFindWindDifficultyIndex(Difficulty);
if (!PerDifficultyProperties.IsValidIndex(Index))
    return false;
FFortGlobalWindInfo& Wind = PerDifficultyProperties[Index].OverrideGlobalWindInfo;
if (Wind.ValidWindInfos.Num() == 0)
{
    return false;
}
if (Wind.WindIndex == INDEX_NONE)
    Wind.WindIndex = FMath::RandHelper(Wind.ValidWindInfos.Num());
OutWind = Wind.ValidWindInfos[Wind.WindIndex];
return true;
