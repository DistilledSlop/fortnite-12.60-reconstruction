// CONFIDENCE: HIGH; captured native behavior and reflection agree.
// Preserve the original class lookups and conditional cast path before the empty call.
if (AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(OtherActor))
{
    if (AController* Controller = Pawn->GetController())
    {
        (void)Cast<AFortPlayerControllerZone>(Controller);
    }
}
