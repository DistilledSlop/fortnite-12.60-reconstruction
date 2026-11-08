// Native helper 0x141a07d50 (static helper on UFortNiagaraGameplayLibrary; callers SpawnFXAtLocation 0x141ddd110, SpawnFXSystemAttached 0x141ddd300).
// Asm: CALL UParticleSystem::StaticClass (0x1447aaa50); then the inline IsA test on RBX through its class at [RBX+0x10]; AL = 1 on success.
// The object is dereferenced without a null check, as in the asm.
// CONFIDENCE: HIGH
return QueryObject->IsA(UParticleSystem::StaticClass());
