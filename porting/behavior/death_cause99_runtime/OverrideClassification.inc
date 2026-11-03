// Uncounted dependency fragment: original 0x141945ed0.
// Requires UFortDeathCauseFromTagMapping and FFortTagToDeathCause headers.
// Preserve output when no row matches. A matched Unspecified still stops scanning.
static void ClassifyDeathCauseOverride(
    const UFortDeathCauseFromTagMapping& Mapping,
    const FGameplayTagContainer& Tags, bool bWasDBNO, EDeathCause& InOutCause)
{
    for (const FFortTagToDeathCause& Row : Mapping.TagToDeathCauseArray)
    {
        if (Tags.HasTag(Row.DeathTag))
        {
            InOutCause = bWasDBNO ? Row.DBNOCause : Row.DeathCause;
            return;
        }
    }
}
