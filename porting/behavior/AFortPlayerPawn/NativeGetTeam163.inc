// CONFIDENCE: high
// Original address: 0x1423253f0
// 0x1423253f0, IFortTeamActorInterface byte slot 0x28.
AFortPlayerStateZone* State = Cast<AFortPlayerStateZone>(GetPlayerState());
uint8 Result = State ? State->NativeGetTeam163() : 1;
if (ControlledRCPawn.IsValid())
{
    Result = static_cast<uint8>(CachedTeamControllingRC);
}
IFortVehicleInterface* Vehicle = NativeGetVehicleInterface163();
if (!State && Vehicle)
{
    // The original explicitly zeroes EDX before this virtual call.
    AFortPlayerStateZone* VehicleState = Vehicle->NativeGetPlayerStateForPawn163(nullptr);
    if (VehicleState)
    {
        Result = VehicleState->NativeGetTeam163();
    }
}
return Result == 0 ? 1 : Result;
