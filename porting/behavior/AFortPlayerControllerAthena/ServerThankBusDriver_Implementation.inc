// CONFIDENCE: MEDIUM
AFortPlayerStateAthena* AthenaPlayerState = Cast<AFortPlayerStateAthena>(PlayerState);
if (!HasAuthority() || !AthenaPlayerState || !AthenaPlayerState->bInAircraft ||
    AthenaPlayerState->bThankedBusDriver)
{
    return;
}

AthenaPlayerState->bThankedBusDriver = true;
AthenaPlayerState->ForceNetUpdate();
