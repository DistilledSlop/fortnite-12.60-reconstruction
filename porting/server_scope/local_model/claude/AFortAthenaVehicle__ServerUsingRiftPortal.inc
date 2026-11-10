// CONFIDENCE: MEDIUM. Original addresses: exec thunk 0x14283d010, virtual slot 0x820 -> 0x1419205d0.
// Only objects whose class implements the VehicleTeleportPortal interface react to a vehicle using them.
if (RiftPortal != nullptr && UVehicleTeleportPortal::StaticClass() != UInterface::StaticClass()
    && RiftPortal->GetClass()->ImplementsInterface(UVehicleTeleportPortal::StaticClass()))
{
    // Helper 0x142b30b20: Blueprint event by FName if present, else native interface call through the interface pointer.
    CallVehicleTeleportPortalEvent(RiftPortal, this);
}
