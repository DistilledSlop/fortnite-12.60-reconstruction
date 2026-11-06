// CONFIDENCE: HIGH
// Original native body 0x141fd72d0.
if (TimeOfDayManager == nullptr)
{
    return NativeSpawnTimeOfDayManager();
}
TimeOfDayManager->Destroy(false, true);
TimeOfDayManager = NativeSpawnTimeOfDayManager();
return TimeOfDayManager;
