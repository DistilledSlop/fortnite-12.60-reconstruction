// CONFIDENCE: HIGH
// Original AFortPawn primary virtual +0x968, 0x14231a880.
if (AdvancedMovementSet)
    return AdvancedMovementSet->MaxAccelerationFlying.GetCurrentValue();
return NativeGetPawnStats141a07b70()->MaxAccelerationFlying;
