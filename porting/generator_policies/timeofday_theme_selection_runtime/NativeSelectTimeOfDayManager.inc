// CONFIDENCE: HIGH
if (Theme.ValidTimeOfDayManagers.Num() == 0)
    NativeRebuildValidTimeOfDayManagers(Theme);
if (Theme.ValidTimeOfDayManagers.Num() == 0)
    return TSoftClassPtr<AFortTimeOfDayManager>();
return Theme.ValidTimeOfDayManagers[FMath::RandHelper(Theme.ValidTimeOfDayManagers.Num())];
