// CONFIDENCE: high; secondary target 0x14084ba80 -> 0x1419452d0.
UFortVehicleSeatComponent* Seats = const_cast<AFortAthenaVehicle*>(this)->NativeGetSeatComponent163();
TArray<AFortPlayerPawn*> Result;
for (APawn* Passenger : Seats->NativeGetAllPassengers163())
{
    Result.Add(static_cast<AFortPlayerPawn*>(Passenger));
}
return Result;
