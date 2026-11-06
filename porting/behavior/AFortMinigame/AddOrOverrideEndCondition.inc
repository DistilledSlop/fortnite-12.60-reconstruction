// CONFIDENCE: high; native implementation 0x142082c90, exec 0x1429bed00.
static_assert(sizeof(FFortMinigameStatQuery) == 0x18, "Native query layout differs");
const auto ValidatedStat = [](const TSubclassOf<UFortMinigameStatFilter>& Stat) { return Stat.Get(); };
for (FMinigameEndCondition& Existing : EndConditionArray)
{
    // TSubclassOf::operator* performs the same subclass validation as native.
    if (ValidatedStat(Existing.StatQuery.Stat) == ValidatedStat(StatQuery.Stat) &&
        Existing.StatQuery.Scope == StatQuery.Scope &&
        Existing.TeamListType == EMinigameTeamListType::Blacklist)
    {
        FMemory::Memcpy(&Existing.StatQuery, &StatQuery, sizeof(FFortMinigameStatQuery));
        return;
    }
}
FMinigameEndCondition& Added = EndConditionArray[EndConditionArray.AddDefaulted()];
FMemory::Memcpy(&Added.StatQuery, &StatQuery, sizeof(FFortMinigameStatQuery));
Added.TeamListType = EMinigameTeamListType::Blacklist;
