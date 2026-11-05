// CONFIDENCE: high; secondary target 0x141917360 -> 0x141f89e10.
UFortVehicleSeatComponent* Seats = const_cast<AFortAthenaVehicle*>(this)->NativeGetSeatComponent163();
return Seats->NativeGetPawnAtSeat163(SeatIdx);
