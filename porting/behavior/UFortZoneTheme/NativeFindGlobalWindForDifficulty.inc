// CONFIDENCE: HIGH
// 0x1425f3c40: selection is cached in the chosen asset entry.
const int32 Index = NativeFindWindDifficultyIndex(Difficulty);
if (!DifficultyProperties.IsValidIndex(Index))
    return false;
FFortGlobalWindInfo& Wind = DifficultyProperties[Index].GlobalWindInfo;
if (Wind.ValidWindInfos.Num() == 0)
{
    OutWind.WindIntensity = 0.0f;
    OutWind.WindHeading = 45.0f;
    return false;
}
if (Wind.WindIndex == INDEX_NONE)
    Wind.WindIndex = FMath::RandHelper(Wind.ValidWindInfos.Num());
OutWind = Wind.ValidWindInfos[Wind.WindIndex];
return true;
