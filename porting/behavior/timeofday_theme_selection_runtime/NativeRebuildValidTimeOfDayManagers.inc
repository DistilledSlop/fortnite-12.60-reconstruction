// CONFIDENCE: HIGH
Theme.ValidTimeOfDayManagers.Empty();
for (const UFortTimeOfDayCollection* Collection : Theme.TimeOfDayCollections)
{
    if (Collection)
        NativeAppendTimeOfDayClasses(Theme.ValidTimeOfDayManagers, Collection->TimeOfDayManagers);
}
NativeAppendTimeOfDayClasses(Theme.ValidTimeOfDayManagers, Theme.AdditionalTimeOfDayManagers);

for (const TSoftClassPtr<AFortTimeOfDayManager>& Prohibited : Theme.ProhibitedTimeOfDayManagers)
{
    for (int32 Index = 0; Index < Theme.ValidTimeOfDayManagers.Num(); ++Index)
    {
        if (Theme.ValidTimeOfDayManagers[Index].ToSoftObjectPath() == Prohibited.ToSoftObjectPath())
        {
            Theme.ValidTimeOfDayManagers.RemoveAtSwap(Index, 1, true);
            --Index;
        }
    }
}
