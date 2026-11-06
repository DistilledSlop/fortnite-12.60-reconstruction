// CONFIDENCE: high; native implementation 0x142082df0, exec 0x1429bedb0.
static_assert(sizeof(FFortMinigameStatQuery) == 0x18, "Native query layout differs");
const auto ValidatedStat = [](const TSubclassOf<UFortMinigameStatFilter>& Stat) { return Stat.Get(); };
const auto QueriesMatch = [&ValidatedStat](const FFortMinigameStatQuery& A, const FFortMinigameStatQuery& B)
{
    return ValidatedStat(A.Stat) == ValidatedStat(B.Stat) && A.Scope == B.Scope &&
        A.bAnyMatch == B.bAnyMatch && A.Operation == B.Operation &&
        A.Value == B.Value && A.bStaticCount == B.bStaticCount;
};
int32 BlacklistIndex = INDEX_NONE;
int32 WhitelistIndex = INDEX_NONE;
for (int32 Index = 0; Index < EndConditionArray.Num(); ++Index)
{
    FMinigameEndCondition& Condition = EndConditionArray[Index];
    if (ValidatedStat(Condition.StatQuery.Stat) != ValidatedStat(StatQuery.Stat) || Condition.StatQuery.Scope != StatQuery.Scope)
    {
        continue;
    }
    if (Condition.TeamListType == EMinigameTeamListType::Blacklist)
    {
        if (QueriesMatch(Condition.StatQuery, StatQuery))
        {
            Condition.TeamList.Remove(TeamIndex);
            BlacklistIndex = Index;
        }
        else
        {
            Condition.TeamList.AddUnique(TeamIndex);
        }
    }
    else if (Condition.TeamListType == EMinigameTeamListType::Whitelist)
    {
        if (QueriesMatch(Condition.StatQuery, StatQuery))
        {
            Condition.TeamList.AddUnique(TeamIndex);
            WhitelistIndex = Index;
        }
        else
        {
            Condition.TeamList.Remove(TeamIndex);
        }
    }
}
if (BlacklistIndex != INDEX_NONE && WhitelistIndex != INDEX_NONE)
{
    return;
}
FMinigameEndCondition& Added = EndConditionArray[EndConditionArray.AddDefaulted()];
FMemory::Memcpy(&Added.StatQuery, &StatQuery, sizeof(FFortMinigameStatQuery));
Added.TeamListType = EMinigameTeamListType::Whitelist;
Added.TeamList.Add(TeamIndex);
