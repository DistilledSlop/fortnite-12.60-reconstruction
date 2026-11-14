// CONFIDENCE: HIGH
// Original AFortPawn virtual +0x948, 0x14231a550.
if (AdvancedMovementSet)
    return AdvancedMovementSet->BrakingDecelerationFalling.GetCurrentValue();
if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
    return Movement->BrakingDecelerationFalling;
return NativeGetPawnStats141a07b70()->BrakingDecelerationFalling;
