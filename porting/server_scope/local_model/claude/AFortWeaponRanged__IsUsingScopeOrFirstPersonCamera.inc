// CONFIDENCE: MEDIUM - control flow read in full; uses the same proposed flag names as IsUsingFirstPersonCamera.
// Original addresses: exec 0x142b4fc40; body 0x1426d29b0
AFortPawn* Pawn = Cast<AFortPawn>(Instigator);
if (Pawn == nullptr)
{
    return false;
}
AFortPlayerController* PC = Cast<AFortPlayerController>(Pawn->FindLocalViewingPlayerController());
if (PC == nullptr || !PC->IsLocalPlayerController())
{
    return false;
}
if (IsUsingFirstPersonFlags(PC) != 0)
{
    return true;
}
return bIsTargeting && bUseScopeTargeting;
