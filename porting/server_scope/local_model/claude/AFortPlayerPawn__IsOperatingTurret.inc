// CONFIDENCE: MEDIUM. Original addresses: exec 0x142a7d350; helper 0x142331940 (read in full). Interface lookup 0x142ec9900 = AActor::GetInterfaceAddress (read in full); UFortVehicleInterface::StaticClass = 0x142b2ca60 (read in full).
// Operating a turret is asked of the vehicle actor's UFortVehicleInterface for the current seat.
if (VehicleStateLocal.Vehicle)
{
    IFortVehicleInterface* TurretInterface = static_cast<IFortVehicleInterface*>(VehicleStateLocal.Vehicle->GetInterfaceAddress(UFortVehicleInterface::StaticClass()));
    if (TurretInterface)
    {
        return TurretInterface->IsSeatOperatingTurret(VehicleStateLocal.SeatIndex);
    }
}
return false;
