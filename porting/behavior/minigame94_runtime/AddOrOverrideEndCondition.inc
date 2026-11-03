// CONFIDENCE: medium; native 0x142082c90. Dependencies documented in README.md.
for (FMinigameEndCondition& Existing : EndConditionArray)
{
    if (Minigame94_ValidatedStatClass(Existing.StatQuery.Stat) ==
            Minigame94_ValidatedStatClass(Query.Stat) &&
        Existing.StatQuery.Scope == Query.Scope &&
        Existing.TeamListType == EMinigameTeamListType::Blacklist)
    {
        Existing.StatQuery = Query;
        return;
    }
}
FMinigameEndCondition NewCondition{};
NewCondition.StatQuery = Query;
NewCondition.TeamListType = EMinigameTeamListType::Blacklist;
EndConditionArray.Add(MoveTemp(NewCondition));
