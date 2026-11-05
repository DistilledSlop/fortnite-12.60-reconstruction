// CONFIDENCE: HIGH
// 0x141fc1750: the first matching ambient entry owns the answer, including
// an empty wind list. The index is selected once and retained in the asset.
for (FWorldAmbientPropertiesMatchingGameContext& Entry : WorldAmbientPropertiesList)
{
    if (!Entry.QueryToMatch.Matches(ContextTags))
        continue;
    FFortGlobalWindInfo& Wind = Entry.GlobalWindInfo;
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
}
return false;
