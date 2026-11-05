// CONFIDENCE: high; complete captured native branches and typed dependencies.
// Native 0x14208fb60; exec 0x1429c0420.
for (const FMinigameEndCondition& Condition : EndConditionArray)
{
    if (Condition.StatQuery.Stat.Get() != InStat.Get()) continue;
    const bool bContains = Condition.TeamList.Contains(TeamNumber);
    if ((Condition.TeamListType == EMinigameTeamListType::Blacklist && !bContains) ||
        (Condition.TeamListType == EMinigameTeamListType::Whitelist && bContains))
    {
        OutValue = Condition.StatQuery.Value;
        return true;
    }
}
return false;
