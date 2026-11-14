// CONFIDENCE: HIGH
// Original AFortPlayerPawn primary virtual +0x968, 0x14231a8b0.
if (AdvancedMovementSet)
    return AdvancedMovementSet->MaxAccelerationFlying.GetCurrentValue();
return NativeGetPawnStats141a07b70()->MaxAccelerationFlying;
