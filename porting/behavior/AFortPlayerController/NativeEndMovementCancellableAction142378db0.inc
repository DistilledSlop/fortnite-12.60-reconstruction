// CONFIDENCE: HIGH. Original142378db0. Zero decrements to -1; do not clamp/count-gate unlock.
if (InMovementCancellableAction >= 0)
{
    --InMovementCancellableAction;
    if (bMovementDisabledDueToCancellableAction)
    {
        bMovementDisabledDueToCancellableAction = false;
        SetIgnoreMoveInput(false);
    }
}
else
{
    UE_LOG(LogFort, Warning, TEXT("Got request to end movement cancellable action when not in one! Ignoring\n"));
}
