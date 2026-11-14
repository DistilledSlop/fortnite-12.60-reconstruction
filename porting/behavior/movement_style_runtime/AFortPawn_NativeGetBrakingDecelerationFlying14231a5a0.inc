// CONFIDENCE: HIGH
// Original AFortPawn virtual +0x950, 0x14231a5a0.
if (AdvancedMovementSet)
    return AdvancedMovementSet->BrakingDecelerationFlying.GetCurrentValue();
if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
    return Movement->BrakingDecelerationFlying;
return NativeGetPawnStats141a07b70()->BrakingDecelerationFlying;
