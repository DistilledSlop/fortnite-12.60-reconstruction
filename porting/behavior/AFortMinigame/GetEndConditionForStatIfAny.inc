// CONFIDENCE: high; complete captured native branches and typed dependencies.
// Native 0x14208f9f0; exec 0x1429c02b0.
for (const FMinigameEndCondition& Condition : EndConditionArray)
{
    if (Condition.StatQuery.Stat.Get() != InStat.Get()) continue;
    const bool bContains = Condition.TeamList.Contains(TeamNumber);
    if ((Condition.TeamListType == EMinigameTeamListType::Blacklist && !bContains) ||
        (Condition.TeamListType == EMinigameTeamListType::Whitelist && bContains))
    {
        static_assert(sizeof(FFortMinigameStatQuery) == 0x18, "Native stat query layout differs");
        FMemory::Memcpy(&OutEndCondition.StatQuery, &Condition.StatQuery, sizeof(FFortMinigameStatQuery));
        OutEndCondition.TeamListType = Condition.TeamListType;
        OutEndCondition.TeamList = Condition.TeamList;
        return true;
    }
}
return false;
