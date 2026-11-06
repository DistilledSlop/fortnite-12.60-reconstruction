// CONFIDENCE: high
// Original address: 0x14160eb90
// 0x14160eb90. Occupancy filters apply only when the requester occupies a vehicle
// and that vehicle's seat participates in the candidate vehicle's filtering.
IFortVehicleInterface* Vehicle = Cast<IFortVehicleInterface>(Candidate);
if (!Vehicle)
{
    AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Candidate);
    Vehicle = Pawn ? Pawn->NativeGetVehicleInterface163() : nullptr;
}
if (!Vehicle)
{
    return false;
}
AFortPlayerPawn* RequesterPawn = Cast<AFortPlayerPawn>(Requester);
if (!RequesterPawn)
{
    return false;
}
IFortVehicleInterface* RequesterVehicle = RequesterPawn->NativeGetVehicleInterface163();
if (!RequesterVehicle || !Vehicle->NativeIsSeatFilterApplicable163(RequesterPawn->GetVehicleSeatIndex()))
{
    return false;
}
if (Filter.bExcludeAthenaVehicleOccupiedBySource && RequesterVehicle == Vehicle &&
    (Cast<IFortVehicleInterface>(Candidate) || Cast<IFortVehicleInterface>(Requester)))
{
    return true;
}
const bool bOccupied = Vehicle->NativeContainsPlayers163();
return (Filter.bExcludeAthenaVehicleOccupied && bOccupied) ||
    (Filter.bExcludeAthenaVehicleUnoccupied && !bOccupied) || !Vehicle->NativeCanPassengersTakeDamage163();
