// CONFIDENCE: MEDIUM - control flow read in full; the spectator state field (+0x2670 on the spectating controller) is a proposed member name.
// Original addresses: exec 0x142b50320; body 0x1426e77c0
AFortPawn* Pawn = Cast<AFortPawn>(Instigator);
if (Pawn != nullptr)
{
    AFortPlayerControllerSpectating* Spectator = Cast<AFortPlayerControllerSpectating>(Pawn->FindLocalViewingPlayerController());
    if (Spectator != nullptr && Spectator->SpectatorModeState != 2)
    {
        return false;
    }
}
return bUseScopeTargeting;
