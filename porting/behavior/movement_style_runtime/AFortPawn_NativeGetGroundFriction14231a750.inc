// CONFIDENCE: HIGH
// Original AFortPawn virtual +0x938, 0x14231a750.
if (AdvancedMovementSet)
    return AdvancedMovementSet->GroundFriction.GetCurrentValue();
if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
    return Movement->GroundFriction;
return NativeGetPawnStats141a07b70()->GroundFriction;
