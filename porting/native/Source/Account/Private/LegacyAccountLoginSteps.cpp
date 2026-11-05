#include "LegacyAccountLoginSteps.h"

void AppendLegacyAccountLoginStep(TArray<FLegacyAccountLoginStep>& Steps,
    const FLegacyAccountLoginStep& Step)
{
    // TArray constructs the new entry; FSimpleDelegate's copy operation clones
    // its delegate instance, including an expired weak binding, without invoking it.
    Steps.Add(Step);
}

void InsertLegacyAccountLoginStep(TArray<FLegacyAccountLoginStep>& Steps,
    FName Anchor, bool bInsertAfter, const FLegacyAccountLoginStep& Step)
{
    const int32 Index = Steps.IndexOfByPredicate([Anchor](const FLegacyAccountLoginStep& Existing)
    {
        return Existing.State == Anchor;
    });
    if (Index != INDEX_NONE)
        Steps.Insert(Step, Index + (bInsertAfter ? 1 : 0));
}
