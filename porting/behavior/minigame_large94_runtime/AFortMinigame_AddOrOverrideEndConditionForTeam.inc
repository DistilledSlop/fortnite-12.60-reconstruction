// Native impl 0x142082df0 (this, uint8 TeamIndex, const FFortMinigameStatQuery* StatQuery). Reflected UFunction
// params: StatQuery (frame +8), TeamIndex (frame +0). SOURCE-ONLY LIFT, NOT CLOSED: two helpers below are unresolved.
// EndConditionArray = this+0x678 (num +0x680, max +0x684), element FMinigameEndCondition size 0x30:
//   +0x00 StatQuery (24 bytes), +0x18 TeamListType (0 Blacklist, 1 Whitelist), +0x20 TeamList (TArray<uint8>).
//
// Unresolved (declared in Minigame94EndConditionBindings.inc, NOT implemented):
//   Minigame94_StatQueryMatches  0x1420802c0 (called with the existing *condition* and the query)
//   Minigame94_EmptyTeamList     0x1420a1cb0 (called on &Condition.TeamList; Empty vs Reset not evidenced)
void AFortMinigame::AddOrOverrideEndConditionForTeam(FFortMinigameStatQuery StatQuery, uint8 TeamIndex)
{
    int32 OverriddenIndex = INDEX_NONE; // native iStackX_8: blacklist entry whose query matched (list emptied)
    int32 AddedIndex = INDEX_NONE;      // native iVar14: whitelist entry whose query matched (team ensured)

    for (int32 Index = 0; Index < EndConditionArray.Num(); ++Index)
    {
        FMinigameEndCondition& Condition = EndConditionArray[Index];

        // Native validates both Stat classes against 0x1429d45e0 (UFortMinigameStatFilter) before comparing, so two
        // invalid classes compare equal; Scope is byte +8.
        if (Minigame94_ValidatedStatClass(Condition.StatQuery.Stat) != Minigame94_ValidatedStatClass(StatQuery.Stat) ||
            Condition.StatQuery.Scope != StatQuery.Scope)
        {
            continue;
        }

        if (Condition.TeamListType != EMinigameTeamListType::Blacklist)
        {
            if (Condition.TeamListType == EMinigameTeamListType::Whitelist)
            {
                if (!Minigame94_StatQueryMatches(Condition, StatQuery))
                {
                    Minigame94_EmptyTeamList(Condition.TeamList);
                }
                else
                {
                    Condition.TeamList.AddUnique(TeamIndex); // linear find, append only when absent
                    AddedIndex = Index;
                }
            }
            // any other TeamListType value: no action, no index recorded
            continue;
        }

        // Blacklist
        if (!Minigame94_StatQueryMatches(Condition, StatQuery))
        {
            Condition.TeamList.AddUnique(TeamIndex);
        }
        else
        {
            Minigame94_EmptyTeamList(Condition.TeamList);
            OverriddenIndex = Index;
        }
    }

    // Native returns only when BOTH indices were set (checked only if the array was non-empty; vacuous otherwise).
    if (OverriddenIndex != INDEX_NONE && AddedIndex != INDEX_NONE)
    {
        return;
    }

    // Append: zero-init, default bitfields, copy all 24 query bytes, TeamListType = 1 (Whitelist), TeamList = {TeamIndex}.
    FMinigameEndCondition NewCondition{};
    NewCondition.StatQuery = StatQuery;
    NewCondition.TeamListType = EMinigameTeamListType::Whitelist;
    NewCondition.TeamList.Add(TeamIndex);
    EndConditionArray.Add(MoveTemp(NewCondition));
}
