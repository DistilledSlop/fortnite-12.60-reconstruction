// CONFIDENCE: high
// Native target 0x1423257c0.
TScriptInterface<IFortVehicleInterface> Result;
if (AActor* Vehicle = VehicleStateLocal.Vehicle)
{
    IFortVehicleInterface* Interface = static_cast<IFortVehicleInterface*>(Vehicle->GetInterfaceAddress(UFortVehicleInterface::StaticClass()));
    if (Interface)
    {
        Result.SetObject(Vehicle);
        Result.SetInterface(Interface);
    }
}
return Result;
