// CONFIDENCE: MEDIUM. Original address: 0x1427db660 (exec thunk). Helper 0x1418f3820 (this, Vehicle).
// Behaviour: restore the vehicle's gravity multiplier to 1.0 (0x3f800000) when it leaves the zone.
if (Vehicle != nullptr)
{
    if (IFortVehicleInterface* VehicleInterface = Cast<IFortVehicleInterface>(Vehicle))
    {
        VehicleInterface->SetGravityMultiplier(1.0f);
    }
}
