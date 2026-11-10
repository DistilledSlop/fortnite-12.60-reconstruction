// CONFIDENCE: LOW. Original address: 0x142831600 (exec thunk). The body calls slot 0 of the vtable of the object embedded at
// this+0x800 (passing that member's own address as this), with the teleported vehicle as argument.
// The member is not reflected and its class is not identified; it is named VehicleTeleportNotifier (see deps).
VehicleTeleportNotifier.OnVehicleTeleported(VehicleActor);
