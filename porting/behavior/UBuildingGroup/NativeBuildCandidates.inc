// CONFIDENCE: HIGH; complete original algorithm under native state preconditions.
// 0x141bdca70. This function replaces, rather than appends to, OutCandidates.
OutCandidates.Reset();
for (const FFortMapData& MapData : BuildingLevelData)
{
    if (MapData.BuildingWorld.ToSoftObjectPath().GetAssetPathName().IsNone())
        continue;

    if (!Filter.IsEmpty())
    {
        FGameplayTagContainer CombinedTags;
        CombinedTags.AppendTags(AllBuildingLevelTags);
        CombinedTags.AppendTags(MapData.BuildingLevelTags);
        if (!Filter.Matches(CombinedTags))
            continue;
    }

    FFoundationNativeCandidate& Candidate = OutCandidates.Emplace_GetRef();
    Candidate.BuildingWorld = MapData.BuildingWorld;
    const FString WorldString = NativeFoundationWorldString_141ed80a0(Candidate.BuildingWorld);
    Candidate.PackageName = FName(*WorldString);
    Candidate.SelectionWeight = MapData.SelectionWeight;
}
