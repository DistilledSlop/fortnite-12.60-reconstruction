// CONFIDENCE: HIGH
// Original AFortPawn virtual +0x990, 0x14231a8f0.
if (AdvancedMovementSet)
    return AdvancedMovementSet->MinAnalogWalkSpeed.GetCurrentValue();
if (const UCharacterMovementComponent* Movement = GetCharacterMovement())
    return Movement->MinAnalogWalkSpeed;
return NativeGetPawnStats141a07b70()->MinAnalogWalkSpeed;
