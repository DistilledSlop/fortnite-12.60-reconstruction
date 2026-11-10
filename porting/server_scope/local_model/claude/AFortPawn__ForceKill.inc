// CONFIDENCE: MEDIUM
// Original address: 0x142a1b7b0 (exec thunk), native helper 0x1423178a0 (dep AFortPawn__ForceKillImpl)
// Script read order (per thunk, matches the helper's parameter use): DeathReason (FGameplayTag, 8 bytes),
// KillerController (object), KillerActor (object). Reflection lists them in reverse declaration order.
ForceKillImpl(DeathReason, KillerController, KillerActor);
