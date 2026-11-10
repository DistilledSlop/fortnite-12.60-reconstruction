// CONFIDENCE: MEDIUM. Non-authority copies do not track team goals (the decompiled scan has no observable effect and the function returns -1).
// Authority: rebuild the goal table for the team and return the goal of the active input item, or -1 when that item has no entry.
// Original address: 0x142747570 (exec). Native body: 0x141bdf360.
if (GetLocalRole() != ROLE_Authority)
{
    return -1;
}
const TArray<FItemGoalEntry> GoalTable = BuildItemGoalTable(InTeam);   // native 0x141bdc280
for (const FItemGoalEntry& Entry : GoalTable)
{
    if (Entry.Item == ActiveInputItem)                                  // ActiveInputItem at +0x820
    {
        return Entry.Goal;                                              // int at entry+0xc
    }
}
return -1;
