// CONFIDENCE: MEDIUM. Original addresses: exec 0x142a7d140; helper 0x142331230 (read in full, depth two).
// Movement part: blocked when pawn+0x10e9 bit 4 is set or CharacterMovement->IsEmoteMovementBlocked() (vtable 0x548, LOW name) is true.
// Vehicle part: blocked when the seat does not allow emotes (vehicle interface slot 0x2b8, LOW name).
// Custom-mode part: MOVE_Custom with custom mode EFortCustomMovement::SurfaceSwimming (10) always blocks.
// Original reads CharacterMovement without a null check; the null guard below changes behaviour only where the original would fault.
bool bBlocked = false;
UFortCharacterMovement* Movement = CharacterMovement;
if (Movement)
{
    const bool bMovementFlagSet = (NativeRecoveredByte_0x10e9 & 0x10) != 0;
    const bool bMovementVirtualBlocked = Movement->IsEmoteMovementBlocked();
    bBlocked = bMovementFlagSet || bMovementVirtualBlocked;
}
IFortVehicleInterface* VehicleInterface = GetVehicleInterface();
if (VehicleInterface)
{
    const bool bSeatAllowsEmote = VehicleInterface->IsSeatEmoteAllowed(VehicleStateLocal.SeatIndex);
    if (!bSeatAllowsEmote)
    {
        bBlocked = true;
    }
}
if (Movement && Movement->MovementMode == MOVE_Custom && Movement->CustomMovementMode == EFortCustomMovement::SurfaceSwimming)
{
    bBlocked = true;
}
return bBlocked;
