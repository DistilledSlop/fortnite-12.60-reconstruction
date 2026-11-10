// CONFIDENCE: HIGH
// Original: exec thunk 0x142a83da0; native body 0x141a24940 (dep: deps/AFortPlayerPawnAthena__SetFallInstigator__SetFallInstigatorInternal.json).
// Reflection params: NewFallingDamageTags (Struct, 0), NewFallInstigator (Object, 1). The thunk reads the instigator first, then the tag container;
// it copies the tag container into a local (FGameplayTagContainer copy-assign) and forwards (this, NewFallInstigator, copy).
SetFallInstigatorInternal(NewFallInstigator, NewFallingDamageTags);
