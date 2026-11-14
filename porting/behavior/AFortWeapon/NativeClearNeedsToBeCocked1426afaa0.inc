// CONFIDENCE: HIGH. Complete original body; broader lifecycle remains open.
// Native 0x1426afaa0. The original only clears the handles when it has a world.
NativeNeedsToBeCocked = false;
if (UWorld* World = GetWorld())
{
    FTimerManager& TimerManager = World->GetTimerManager();
    TimerManager.ClearTimer(NativeCockingTimerB30);
    TimerManager.ClearTimer(NativeCockingTimerB38);
}
UE_LOG(LogFort, Log, TEXT("Needs to be cocked cleared"));
