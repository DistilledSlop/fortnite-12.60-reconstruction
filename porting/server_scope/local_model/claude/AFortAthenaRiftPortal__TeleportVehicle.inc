// CONFIDENCE: LOW. Original address: 0x142831c30 (exec thunk). Helper chain: 0x1418fc3b0 -> 0x141a8e150 (captured, read in full).
// Behaviour: a vehicle is teleported only when it is a FortAthenaVehicle that is in a teleportable state, and the vehicle
// lies within the portal radius (+1 unit) of this portal. On success the vehicle is teleported to TeleportLocation and the
// vehicle is notified through its UFunction (name unresolved, see notes).
if (Vehicle == nullptr)
{
    return;
}
if (!Vehicle->IsA(AFortAthenaVehicle::StaticClass()))
{
    return;
}

// Teleportable: (state == 3 and not blocked) or the vehicle reports ready (vslot 0x558).
const bool bStateAllows = (Vehicle->TeleportState == 3) && !Vehicle->IsTeleportBlocked();   // vslot 0x3f8 negated
if (!bStateAllows && !Vehicle->IsReadyForTeleport())                                           // vslot 0x558
{
    return;
}

// Distance check in the XY plus Z of the portal root and vehicle root positions.
const FVector VehicleLocation = (Vehicle->RootComponent != nullptr) ? Vehicle->RootComponent->RelativeLocation : FVector(DefaultLocation);
const FVector PortalLocation  = (RootComponent != nullptr) ? RootComponent->RelativeLocation : FVector(DefaultLocation);
const FVector Delta = PortalLocation - VehicleLocation;

float PortalRadius = 0.0f;
GetPortalRadius(PortalRadius);                                                  // vslot 0x560 on the portal

if (Delta.SizeSquared() < (PortalRadius + 1.0f) * (PortalRadius + 1.0f))
{
    Vehicle->TeleportTo(TeleportLocation);                                      // vslot 0x7c8 on the vehicle
    Vehicle->ProcessEvent(FindFunctionChecked(VehicleTeleportedThroughPortal_FunctionName), this);   // FUN_142839350
}
