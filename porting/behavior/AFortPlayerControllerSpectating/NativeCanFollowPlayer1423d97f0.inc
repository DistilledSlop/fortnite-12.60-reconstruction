// CONFIDENCE: HIGH. Original1423d97f0; original GUObjectArray bit29 is PendingKill.
if (!State || State->IsPendingKill() || static_cast<uint8>(State->PlayerRole) != 0)
{
    return false;
}
AFortPlayerStateAthena* AthenaState = Cast<AFortPlayerStateAthena>(State);
if ((!AthenaState || !AthenaState->bInAircraft) && !State->GetCurrentPawn())
{
    return false;
}
if (AthenaState && !AthenaState->NativeAllowsReplaySpectating216())
{
    return false;
}
if (State->GetCurrentPawn() && State->GetCurrentPawn()->IsDead()
    && State->GetCurrentPawn()->GetExtraLives() == 0)
{
    return false;
}
return true;
