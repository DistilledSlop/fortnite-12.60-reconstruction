// CONFIDENCE: MEDIUM. Starts a deferred spawn of the projectile class at the task's spawn transform (location from SpawnLocation, rotation from SpawnRotation, unit scale), binds the projectile events back to this task, and returns the not-yet-finished actor through SpawnedActor. Returns false when the ability, the world or the class is unusable.
// Original addresses: exec thunk 0x142788de0; body helper 0x14161ed10.
SpawnedActor = nullptr;

if (!Ability || !IsAbilityReadyToSpawn(Ability))   // helper 0x140718920 on Ability+0x380 (activation gate, name not recovered)
{
	return false;
}
UWorld* World = GEngine->GetWorldFromContextObject(OwningAbility, EGetWorldErrorMode::LogAndReturnNull);   // helper 0x144640410
if (!World)
{
	return false;
}

UClass* SpawnClass = nullptr;
if (ProjectileClass && ProjectileClass->IsChildOf(AFortProjectileBase::StaticClass()))   // helper 0x142aa44e0
{
	SpawnClass = ProjectileClass;
}
if (!SpawnClass)
{
	return false;
}

const FTransform SpawnTransform(SpawnRotation.Quaternion(), SpawnLocation, FVector(1.0f, 1.0f, 1.0f));
FActorSpawnParameters SpawnParams;
SpawnParams.Owner = RequestedBy;   // proposed mapping of the helper's spawn-parameter fields
SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;   // flag bit 0x4 set by helper

AActor* Spawned = World->SpawnActorDeferred<AActor>(SpawnClass, SpawnTransform, SpawnParams);   // helper 0x1442e6580
if (!Spawned || !Spawned->IsA(AFortProjectileBase::StaticClass()))
{
	return false;
}

SpawnedActor = Spawned;
BindSpawnedProjectileEvents(Spawned, this);   // inline section of helper 0x14161ed10 (see deps)
return true;
