// CONFIDENCE: HIGH
// Original: exec thunk 0x142a81000; native body 0x14235bd20 (dep: deps/AFortPlayerPawn__UpdatePreviousPositionAndVelocity__UpdatePreviousPositionAndVelocityInternal.json).
// Reflection params: OldVelocity (Struct, 0), OldLocation (Struct, 1), DeltaSeconds (Float, 2). The thunk reads DeltaSeconds, OldLocation, OldVelocity
// in that order and forwards (this, DeltaSeconds, &OldLocation, &OldVelocity).
// The native body stores OldLocation into the previous-position slot (+0x1168), OldVelocity into PreviousVelocity (+0x1174)
// and then calls virtual slot 0x1008, which resolves to the no-op stub 0x1406801c0 (STUB). DeltaSeconds is not used by the native body.
UpdatePreviousPositionAndVelocityInternal(DeltaSeconds, OldLocation, OldVelocity);
