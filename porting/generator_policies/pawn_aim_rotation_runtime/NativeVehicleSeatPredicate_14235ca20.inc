// CONFIDENCE: HIGH. Recovered original pawn/vehicle virtual family.
// AFortPlayerPawn helper: bool NativeVehicleSeatPredicate_14235ca20() const.
// Original 0x14235ca20, UFortVehicleInterface::StaticClass 0x142b2ca60,
// UObjectBaseUtility::GetInterfaceAddress 0x142ec9900.
if (!VehicleStateLocal.Vehicle)
    return false;
IFortVehicleInterface* Vehicle = Cast<IFortVehicleInterface>(VehicleStateLocal.Vehicle);
return Vehicle ? Vehicle->NativeSeatPredicate_00C8(VehicleStateLocal.SeatIndex) : false;
