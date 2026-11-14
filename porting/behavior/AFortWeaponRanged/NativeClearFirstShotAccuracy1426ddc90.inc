// CONFIDENCE: HIGH. Original1426ddc90; invalidate the handle through the timer manager.
NativeFirstShotAccuracyReady = false;
if (NativeFirstShotAccuracyTimer.IsValid())
{
    GetWorldTimerManager().ClearTimer(NativeFirstShotAccuracyTimer);
}
