// CONFIDENCE: HIGH
// Original AFortPawn virtual +0x958, 0x14231a640.
if (AdvancedMovementSet)
    return AdvancedMovementSet->BrakingFrictionFactor.GetCurrentValue();
if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
    return Movement->BrakingFrictionFactor;
return NativeGetPawnStats141a07b70()->BrakingFrictionFactor;
