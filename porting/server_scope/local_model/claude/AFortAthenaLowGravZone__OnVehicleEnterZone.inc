// CONFIDENCE: MEDIUM. Original address: 0x1427db590 (exec thunk). Helper 0x1418f37d0 (this, Vehicle, GravityMultiplier).
// Behaviour: when the vehicle is valid, resolve its IFortVehicleInterface and apply the zone gravity multiplier.
// The interface slot (vtable +0x6a0) is an abstract IFortVehicleInterface method; its proposed name is
// IFortVehicleInterface::SetGravityMultiplier (see deps/AFortAthenaLowGravZone_IFortVehicleInterface_SetGravityMultiplier.json).
if (Vehicle != nullptr)
{
    if (IFortVehicleInterface* VehicleInterface = Cast<IFortVehicleInterface>(Vehicle))
    {
        VehicleInterface->SetGravityMultiplier(GravityMultiplier);
    }
}
