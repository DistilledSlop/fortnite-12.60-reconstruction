// CONFIDENCE: HIGH. Original 0x141be60c0; native sequence lifecycle is required.
TArray<FFoundationNativeCandidate> Candidates;
if (Sequence)
{
    NativeCachedBuildingGroup = NativeResolveBuildingGroup(Sequence);
    if (NativeCachedBuildingGroup)
    {
        TArray<FFoundationNativeCandidate> GroupCandidates;
        NativeCachedBuildingGroup->NativeBuildCandidates(GroupCandidates, BuildingGroupFilter);
        Candidates.Reserve(GroupCandidates.Num() + AdditionalWorlds.Num());
        Candidates.Append(GroupCandidates);
    }
}
Candidates.Reserve(Candidates.Num() + AdditionalWorlds.Num());
for (const TSoftObjectPtr<UWorld>& World : AdditionalWorlds)
{
    const FString WorldString = NativeFoundationWorldString_141ed80a0(World);
    if (!WorldString.IsEmpty())
    {
        FFoundationNativeCandidate& Candidate = Candidates.Emplace_GetRef();
        Candidate.BuildingWorld = World;
        Candidate.PackageName = FName(*WorldString);
        Candidate.SelectionWeight = 1.0f;
    }
}
NativeFilterBuildingCandidates(Candidates);
if (Candidates.Num() == 0)
{
    if (UE_LOG_ACTIVE(LogFortWorld, Log))
    {
        const FString GroupName = BuildingGroups.Num() > 0 && BuildingGroups[0]
            ? BuildingGroups[0]->GetPathName() : FString(TEXT("?"));
        UE_LOG(LogFortWorld, Log, TEXT("BUILDING ERROR! (%s.%s) could not select a building!  No more buildings found to use! (Name: %s, filter: %s) "),
            *GetLevel()->GetFName().ToString(), *GetFName().ToString(),
            *GroupName, *BuildingGroupFilter.GetDescription());
    }
    return NAME_None;
}
if (Candidates.Num() == 1)
    return Candidates[0].PackageName;
if (!Sequence)
    return NAME_None;

const TArray<FFoundationNativeCandidate> OriginalCandidates = Candidates;
float TotalWeight = 0.0f;
bool bSameWeights = true;
if (AFortWorldManager* Manager = AFortWorldManager::NativeResolveWorldManager(GetWorld()))
{
    for (int32 Index = Candidates.Num() - 1; Index >= 0; --Index)
    {
        const float Weight = Candidates[Index].SelectionWeight;
        if (Weight <= 0.0f || Manager->NativeHasUsedBuilding(Candidates[Index].PackageName))
        {
            Candidates.RemoveAt(Index);
        }
        else
        {
            // Compare with the current first element, even before it is filtered.
            if (Weight != Candidates[0].SelectionWeight)
                bSameWeights = false;
            TotalWeight += Weight;
        }
    }
}
if (Candidates.Num() == 0 || TotalWeight <= 0.0f)
{
    const int32 Index = int32((float(OriginalCandidates.Num()) - 0.00001f) * Sequence->Next());
    return OriginalCandidates.GetData()[Index].PackageName;
}
if (bSameWeights)
{
    const int32 Index = int32((float(Candidates.Num()) - 0.00001f) * Sequence->Next());
    return Candidates.GetData()[Index].PackageName;
}
float RemainingWeight = Sequence->Next() * TotalWeight;
int32 Index = 0;
while (Index < Candidates.Num() - 1)
{
    if (RemainingWeight < Candidates[Index].SelectionWeight)
        break;
    RemainingWeight -= Candidates[Index].SelectionWeight;
    ++Index;
}
return Candidates.GetData()[Index].PackageName;
