// CONFIDENCE: HIGH
// Original AFortPawn virtual +0x940, 0x14231a5f0.
if (AdvancedMovementSet)
    return AdvancedMovementSet->BrakingDecelerationWalking.GetCurrentValue();
if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
    return Movement->BrakingDecelerationWalking;
return NativeGetPawnStats141a07b70()->BrakingDecelerationWalking;
