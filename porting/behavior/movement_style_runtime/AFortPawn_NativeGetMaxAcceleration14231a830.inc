// CONFIDENCE: HIGH
// Original AFortPawn virtual +0x960, 0x14231a830.
if (AdvancedMovementSet)
    return AdvancedMovementSet->MaxAcceleration.GetCurrentValue();
if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
    return Movement->MaxAcceleration;
return NativeGetPawnStats141a07b70()->MaxAcceleration;
