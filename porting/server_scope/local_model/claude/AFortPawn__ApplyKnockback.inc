// CONFIDENCE: MEDIUM
// Original address: 0x142a1b200 (exec thunk). Reads KnockbackMagnitude (raw offset 0), KnockbackZAngle (raw offset 4) and ImpulseDir (raw offset 8) in that order, then forwards them to the native body 0x1423055b0 (deps AFortPawn__ApplyKnockbackImpl).
ApplyKnockbackImpl(KnockbackMagnitude, KnockbackZAngle, ImpulseDir);
