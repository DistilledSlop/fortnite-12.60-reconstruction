// CONFIDENCE: HIGH
// Original: exec thunk 0x142a20030; native body 0x14234dac0.
// bIsKnockedback is the 1-bit bitfield in byte +0xa4 (the native code writes bit 6 of that byte).
// The dispatch through vtable slot 0x9f0 on AFortPawn resolves to the shared empty stub 0x1406801c0 (ret 0),
// so the base OnRep_IsKnockedBack body does nothing.
bIsKnockedback = bKnockback;
OnRep_IsKnockedBack();
