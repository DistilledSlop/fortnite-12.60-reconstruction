// CONFIDENCE: MEDIUM. Completes a deferred projectile spawn. Finishes the actor at the task spawn transform, records it as SpawnedProj, applies the spawn direction and initial speed, gravity scale and requester, and appends a spawn record to the task. A null or destroyed actor is rejected with an ensure.
// Original addresses: exec thunk 0x142789000; body helper 0x141625fd0.
if (!SpawnedActor)
{
	ensure(false);
	return;
}

const FTransform SpawnTransform(SpawnRotation.Quaternion(), SpawnLocation, FVector(1.0f, 1.0f, 1.0f));
SpawnedActor->FinishSpawning(SpawnTransform);   // helper 0x143f71060

if (!IsValid(SpawnedActor) || SpawnedActor->IsActorBeingDestroyed())
{
	ensure(false);
	return;
}

SpawnedProj = SpawnedActor;                                  // TWeakObjectPtr at +0xe0
AFortProjectileBase* Projectile = Cast<AFortProjectileBase>(SpawnedProj.Get());
InitializeProjectileVelocity(Projectile, SpawnDirection.Quaternion(), InitialSpeed);   // helper 0x1426df7d0 (direction from +0x108, speed from +0x114)
SetProjectileGravityScale(Projectile, GravityScale);                                    // helper 0x1426df650 (gravity from +0x288)
NotifyProjectileRequestedBy(Projectile, Projectile, RequestedBy);                      // helper 0x142aaab30 (ProcessEvent plus interface call)
SpawnedProjectileRecords.Add(FSpawnedProjectileRecord{ SpawnedActor });                // helper 0x14161f450 on this+0x80
