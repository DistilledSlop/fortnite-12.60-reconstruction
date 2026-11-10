// CONFIDENCE: MEDIUM
// Original address: 0x142a7ace0 (exec thunk). Thunk reads InActorToIgnore (object) then InIgnoreDuration (float), then calls FUN_142302250(this, actor, duration).
// Captures: FUN_142302250 captures/4dea1c9b9f9244939a814d8f3cd4bfa2/0x142302250.c ; refresh helper FUN_142347f20 captures/.../0x142347f20.c (read in full) ; FUN_1440f4040 (read in full).
// Depth note: FUN_144621fb0, FTimerManager_InternalClearTimer, FUN_1446278d0 (timer set) and FUN_14072c6c0 are NOT read; they are described only.
UWorld* World = GetWorld();
if (World == nullptr)
{
	return;
}
float Now = World->TimeSeconds;   // +0x540

// Ignore list (TArray at +0x1c70 of the pawn, count +0x1c78, 16-byte entries {Actor, StartTime, RemainingAtStart}).
for (FIgnoredActorEntry& Entry : IgnoredActors)
{
	if (Entry.Actor == InActorToIgnore)
	{
		if (Entry.RemainingAtStart - (Now - Entry.StartTime) <= InIgnoreDuration)
		{
			return;   // existing ignore already lasts at least as long
		}
		Entry.RemainingAtStart = InIgnoreDuration;
		Entry.StartTime = Now;
		RefreshIgnoredActors();   // FUN_142347f20
		return;
	}
}
// New entry: register with the collision/ignore component (this+0x288, FUN_1440f4040 with bAdd = true).
if (IgnoreComponent != nullptr)
{
	IgnoreComponent->SetIgnoredActor(InActorToIgnore, true);
}
IgnoredActors.Add(FIgnoredActorEntry{InActorToIgnore, Now, InIgnoreDuration});
RefreshIgnoredActors();   // FUN_142347f20
