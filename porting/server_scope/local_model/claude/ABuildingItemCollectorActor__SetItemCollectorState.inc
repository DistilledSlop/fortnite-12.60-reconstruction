// CONFIDENCE: MEDIUM. Authority-only state change. Leaving state 1 with a controlling player settles that player's deposit goal for the active input item, then the state is stored, notified and refreshed.
// Original address: 0x142747ca0 (exec). Native body: 0x141bea830.
if (GetLocalRole() == ROLE_Authority)
{
    if (ItemCollectorState == 1 && NewState != 1)
    {
        int32 PendingCount = 0;
        if (ControllingPlayer != nullptr)
        {
            const int32 Reserved = FindPlayerItemValue(ControllingPlayer, ActiveInputItem, 2);   // native 0x141bdf6a0, column 2
            if (Reserved != -1)
            {
                PendingCount = Reserved;
            }
        }
        if (ItemCollectorBehavior < 2)   // byte at +0x7b1
        {
            for (FCollectorItemRecord& Record : PlayerItemRecords(ControllingPlayer))   // native 0x141bdf3f0
            {
                if (Record.Item == ActiveInputItem)
                {
                    Record.Count = 0;                                                  // +0x8
                }
            }
            float DepositGoal = GetDepositGoal(ControllingPlayer, ActiveInputItem);   // native 0x141bdd430
            if (const FItemCollection* Collection = FindItemCollection(ActiveInputItem))   // native 0x141bcc640
            {
                DepositGoal = Collection->GoalCurve.GetValueAtLevel(StartingGoalLevel + PendingCount);
            }
            for (FCollectorItemRecord& Record : PlayerItemRecords(ControllingPlayer))
            {
                if (Record.Item == ActiveInputItem)
                {
                    Record.DepositGoal = FMath::Max(0, static_cast<int32>(DepositGoal));   // +0xc
                }
            }
        }
        ControllingPlayer = nullptr;
    }
    ItemCollectorState = NewState;                 // byte at +0x7b3
    OnItemCollectorStateChanged(NewState);         // native 0x1427460a0, BP event with one byte argument
    RefreshItemCollector();                        // native 0x141befa00
}
