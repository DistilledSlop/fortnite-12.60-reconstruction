// CONFIDENCE: HIGH. Full original body.
// Original reflected thunk 0x142a7d110 calls native body 0x142331180.
// The existing virtual getter is not const-qualified in reconstructed declarations.
IFortVehicleInterface* Vehicle = const_cast<AFortPlayerPawn*>(this)->NativeGetVehicleInterface163();
return Vehicle && Vehicle->NativeSeatPredicate_0568(VehicleStateLocal.SeatIndex);
