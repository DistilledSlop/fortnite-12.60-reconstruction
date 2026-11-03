// Original native body 0x141fd72d0. Requires the complete implementation of
// NativeSpawnTimeOfDayManager; this fragment is not a standalone integration.
if (TimeOfDayManager == nullptr)
{
    return NativeSpawnTimeOfDayManager();
}
TimeOfDayManager->Destroy(false, true);
TimeOfDayManager = NativeSpawnTimeOfDayManager();
return TimeOfDayManager;
