// CONFIDENCE: HIGH
// Original AFortPawn virtual +0x978, 0x14231a7e0.
if (AdvancedMovementSet)
    return AdvancedMovementSet->JumpZVelocity.GetCurrentValue();
if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
    return Movement->JumpZVelocity;
return NativeGetPawnStats141a07b70()->JumpZVelocity;
