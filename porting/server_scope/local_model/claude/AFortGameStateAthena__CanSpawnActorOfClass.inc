// CONFIDENCE: MEDIUM. Exec thunk 0x14291e400 reads InstigatorActor, ActorClassToSpawn, Location, Rotation in declaration order and returns the spawn check result from the core helper 0x14198bb80 (copied into ReturnValue via 0x142918000, a move-assign of the result struct).
// Original address: 0x14291e400 (exec). Core: 0x14198bb80 (deps AFortGameStateAthena__CanSpawnActorOfClassCore).
FSpawnCheckResult Result = CanSpawnActorOfClassCore(InstigatorActor, ActorClassToSpawn, Location, Rotation);
return Result;
