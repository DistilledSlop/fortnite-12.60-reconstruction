// CONFIDENCE: HIGH. Original1423d4240; preserve virtual followed-player query and repeated vehicle lookup.
AFortPlayerState* FollowedState = GetFollowedPlayer();
AFortPlayerPawn* FollowedPawn = FollowedState ? FollowedState->GetCurrentPawn() : nullptr;
if (!bIgnoreVehicle && FollowedPawn)
{
    if (FollowedPawn->NativeGetAthenaVehicle1423254c0())
    {
        AFortAthenaVehicle* Vehicle = FollowedPawn->NativeGetAthenaVehicle1423254c0();
        return Vehicle;
    }
}
return FollowedPawn;
