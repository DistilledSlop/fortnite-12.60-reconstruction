// Original body address: 0x141125940 (helper, no thunk)
// Offset evidence: +0x118 AActor::Instigator (reflected Object, APawn*). Asm is one load and return with no null check.
// Callers (ABuildingGameplayActor::SetIgnoreInstigatorCollision and SetInitialVelocity) pass the building actor itself as the first argument, so the parameter is AActor*.
// CONFIDENCE: HIGH
return TargetActor->GetInstigator();
