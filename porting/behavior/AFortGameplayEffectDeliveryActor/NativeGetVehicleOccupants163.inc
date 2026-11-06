// CONFIDENCE: high
// Original address: 0x14160eb90
// 0x14160eb90 vehicle-interface branches; no pawn-to-vehicle conversion here.
IFortVehicleInterface* Vehicle = Cast<IFortVehicleInterface>(Actor);
if (!Vehicle)
{
    return false;
}
Occupants = Vehicle->NativeGetAllPassengers163();
return true;
