// CONFIDENCE: MEDIUM
// Original: exec thunk 0x142a80ed0; native body 0x142358750 (dep: deps/AFortPlayerPawn__UnableToPerformAction__UnableToPerformActionInternal.json).
// Reflection params: FailedReason (Struct, index 0), bPlayMontage (Bool, index 1). The thunk reads bPlayMontage first, then FailedReason,
// and forwards (this, bPlayMontage, FailedReason) to the native body.
UnableToPerformActionInternal(bPlayMontage, FailedReason);
