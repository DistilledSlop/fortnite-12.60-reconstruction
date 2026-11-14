// CONFIDENCE: high. Original 0x14234bf20; virtual pawn slot +0x9a0.
// Deliberately retain the initially captured component for the first two stores.
auto* InitialMovement = GetCharacterMovement();
CurrentMovementStyle = static_cast<EFortMovementStyle::Type>(Style);
if (InitialMovement)
{
    InitialMovement->MaxWalkSpeed = NativeGetMovementSpeed14231ab10(Style);
    InitialMovement->MaxFlySpeed = GetCharacterMovement()->MaxWalkSpeed;
    if (AdvancedMovementSet)
    {
        const float GroundFrictionValue = NativeGetGroundFriction14231a750();
        if (auto* Movement = GetCharacterMovement()) Movement->GroundFriction = GroundFrictionValue;
        const float WalkingValue = NativeGetBrakingDecelerationWalking14231a5f0();
        if (auto* Movement = GetCharacterMovement()) Movement->BrakingDecelerationWalking = WalkingValue;
        const float FallingValue = NativeGetBrakingDecelerationFalling14231a550();
        if (auto* Movement = GetCharacterMovement()) Movement->BrakingDecelerationFalling = FallingValue;
        const float FlyingValue = NativeGetBrakingDecelerationFlying14231a5a0();
        if (auto* Movement = GetCharacterMovement()) Movement->BrakingDecelerationFlying = FlyingValue;
        const float FrictionFactorValue = NativeGetBrakingFrictionFactor14231a640();
        if (auto* Movement = GetCharacterMovement()) Movement->BrakingFrictionFactor = FrictionFactorValue;
        const float AccelerationValue = NativeGetMaxAcceleration14231a830();
        if (auto* Movement = GetCharacterMovement()) Movement->MaxAcceleration = AccelerationValue;
        NativeSetMaxAccelerationFlying141a24e80(NativeGetFlyingAcceleration14231a880());
        const float JumpValue = NativeGetJumpZVelocity14231a7e0();
        if (auto* Movement = GetCharacterMovement()) Movement->JumpZVelocity = JumpValue;
        const float AnalogValue = NativeGetMinAnalogWalkSpeed14231a8f0();
        if (auto* Movement = GetCharacterMovement()) Movement->MinAnalogWalkSpeed = AnalogValue;
    }
}
