// CONFIDENCE: high. Original 0x14234c0a0; Player/Athena virtual slot +0x9a0.
const uint8 PreviousStyle = static_cast<uint8>(CurrentMovementStyle.GetValue());
AFortPawn::NativeSetMovementStyle14234bf20(Style);
if (static_cast<uint8>(CurrentMovementStyle.GetValue()) == 4)
{
    if (PreviousStyle != 4)
    {
        bUseControllerRotationYawToRestore = bUseControllerRotationYaw;
        bUseControllerRotationYaw = false;
        NativePersonalVehicleStyleChanged1608.Broadcast(true);
    }
}
else if (PreviousStyle == 4)
{
    bUseControllerRotationYaw = bUseControllerRotationYawToRestore;
    NativePersonalVehicleStyleChanged1608.Broadcast(false);
}
// Original final call is 0x1406801c0 (empty). Retain branch and packed input
// reads, including its non-null movement precondition.
if (bIsLocalPlayer || NativeLocalMovementNotify2004Bit2)
{
    auto* Movement = GetCharacterMovement();
    uint32 Packed = ((static_cast<uint8>(BuildingState.GetValue()) & 15u) << 4) |
        (static_cast<uint8>(CurrentMovementStyle.GetValue()) & 15u);
    Packed = (Packed << 4) | (Movement->CustomMovementMode & 15u);
    Packed = (Packed << 4) | (static_cast<uint8>(Movement->MovementMode) & 15u);
    const auto OriginalEmpty1406801c0 = [](uint32) {};
    OriginalEmpty1406801c0(Packed);
}
