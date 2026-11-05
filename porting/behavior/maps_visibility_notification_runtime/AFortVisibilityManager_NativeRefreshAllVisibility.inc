// CONFIDENCE: high
// 0x14263c500, subject to the per-record helper's unresolved dependencies.
if (GetLocalRole() == ROLE_Authority)
{
    for (FVisibilityInfo& Item : VisibilityInfoArray.VisibilityInfoArray)
    {
        NativeRefreshVisibilityRecord(Item, true);
    }
}
