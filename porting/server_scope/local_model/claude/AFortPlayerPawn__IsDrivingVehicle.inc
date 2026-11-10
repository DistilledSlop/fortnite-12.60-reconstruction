// CONFIDENCE: MEDIUM. Original addresses: exec 0x142a7d110; helper 0x142331180 (tail-calls a vehicle interface slot, null interface gives false).
// Seat index is VehicleStateLocal.SeatIndex (byte at pawn+0x1584, confirmed by the ported GetVehicleSeatIndex).
IFortVehicleInterface* VehicleInterface = GetVehicleInterface();
if (!VehicleInterface)
{
    return false;
}
return VehicleInterface->IsDriverSeat(VehicleStateLocal.SeatIndex);
