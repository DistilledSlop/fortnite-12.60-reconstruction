// CONFIDENCE: HIGH. Original1423b8b70; counter is native 32-bit wraparound.
NativeSetAutoRun1423ae530(false);
InMovementCancellableAction = static_cast<int32>(static_cast<uint32>(InMovementCancellableAction) + 1u);
if (!bMovementDisabledDueToCancellableAction)
{
    if (!MyFortPawn || !MyFortPawn->bMovingEmote)
    {
        bMovementDisabledDueToCancellableAction = true;
        SetIgnoreMoveInput(true);
    }
}
else
{
    UE_LOG(LogFort, Warning, TEXT("Got request to start movement cancellable action when already in one. InMovementCancellableAction count %d"), InMovementCancellableAction);
}
