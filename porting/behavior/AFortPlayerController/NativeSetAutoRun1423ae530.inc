// CONFIDENCE: HIGH. Original1423ae530; preserve pawn re-reads around virtual calls.
if (bAutoRunOn == bEnable) return;
const bool bDriving = MyFortPawn && MyFortPawn->IsDrivingVehicle();
if (MyFortPawn && MyFortPawn->NativeGetAthenaVehicle1423254c0() && !bDriving)
{
    bEnable = false;
}
bAutoRunOn = bEnable;
if (bEnable)
{
    // Original1482472b8 is GFrameCounter: TimerManager1446278d0 compares it
    // against LastTickedFrame at150 before adding InternalTime at140.
    NativeAutoRunStartFrameAe8 = GFrameCounter;
}
OnAutoRunEnabled.Broadcast(bEnable);
