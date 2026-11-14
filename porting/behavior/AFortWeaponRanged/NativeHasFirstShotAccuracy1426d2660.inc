// CONFIDENCE: HIGH
// Original 0x1426d2660.
if (!NativeUsesFirstShotAccuracy1426cfa80())
{
    return false;
}
const bool bOwnerIsMoving = NativeOwnerIsMoving1426d6940();
if (!bIsTargeting || bOwnerIsMoving)
{
    return false;
}
const float WaitTime = FirstShotAccuracyMinWaitTime;
if (NativeFirstShotAccuracyReady)
{
    return NativeAtMinimumReticleSpread.Get();
}
FTimerManager& TimerManager = GetWorldTimerManager();
if (!TimerManager.IsTimerActive(NativeFirstShotAccuracyTimer))
{
    TimerManager.SetTimer(NativeFirstShotAccuracyTimer, this,
        &AFortWeaponRanged::NativeMarkFirstShotAccuracy1426dd430, WaitTime, false);
}
return false;
