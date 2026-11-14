// CONFIDENCE: HIGH; complete original algorithm under native state preconditions.
// 0x141be5f10; Sequence is a required native caller precondition.
if (NativeCachedBuildingGroup != nullptr)
{
    UBuildingGroup* Group = NativeCachedBuildingGroup;
    while (Group->FallbackGroup != nullptr && Group->BuildingLevelData.Num() <= 0 &&
           Group->FoundationType == FoundationType)
    {
        Group = Group->FallbackGroup;
    }
    if (Group->BuildingLevelData.Num() > 0 && Group->FoundationType == FoundationType)
    {
        if (Group != NativeCachedBuildingGroup)
            NativeCachedBuildingGroup = Group;
        return Group;
    }
}

TArray<UBuildingGroup*> EligibleGroups;
for (int32 Index = BuildingGroups.Num() - 1; Index >= 0; --Index)
{
    UBuildingGroup* Group = BuildingGroups[Index];
    if (Group != nullptr && Group->BuildingLevelData.Num() > 0 &&
        Group->FoundationType == FoundationType)
    {
        EligibleGroups.Add(Group);
    }
}
if (EligibleGroups.Num() == 0)
    return nullptr;

const int32 Index = static_cast<int32>((static_cast<float>(EligibleGroups.Num()) - 0.00001f) * Sequence->Next());
NativeCachedBuildingGroup = EligibleGroups.GetData()[Index];
return NativeCachedBuildingGroup;
