// CONFIDENCE: MEDIUM. Factory for the projectile-spawn ability task. Logs the request, creates the task, initialises it from the owning ability, copies the spawn parameters and effect container specs, and returns the task.
// Original addresses: exec 0x14278a730; helpers 0x141639ff0 (create and fill), 0x141614350 (effect container spec copy, called for both specs).
UE_LOG(LogTemp, Log, TEXT("UFortAbilityTask_SpawnProjectileAndWait::SpawnProjectileAndWait Spawning a projectile requested by actor: %s, projectile: %s"), *GetNameSafe(RequestedBy), *GetNameSafe(Class));

UFortAbilityTask_SpawnProjectileAndWait* Task = NewObject<UFortAbilityTask_SpawnProjectileAndWait>(GetTransientPackage());
Task->InitTask(*OwningAbility, OwningAbility->GetGameplayTaskDefaultPriority());   // helper 0x144858e40 on the ability's task component
Task->ProjectileClass = Class;                       // +0xe8
Task->SpawnLocation = SpawnLocation;                 // +0xf0
Task->SpawnRotation = SpawnRotation;                 // +0xfc
Task->SpawnDirection = SpawnDirection;               // +0x108
Task->InitialSpeed = InitialSpeed;                   // +0x114
Task->RequestedBy = RequestedBy;                     // +0x290
Task->EffectContainerSpecToApplyOnHit = CopyEffectContainerSpec(EffectContainerSpecToApplyOnHit);     // helper 0x141614350 (order of the two specs not verified)
Task->EffectContainerSpecToApplyOnExplode = CopyEffectContainerSpec(EffectContainerSpecToApplyOnExplode);
Task->GravityScale = GravityScale;                   // +0x288
Task->HomingTarget = HomingTarget;
Task->bAllowSpawnWhenDBNO = bAllowProjectileSpawnWhenDBNO;
Task->bAllowSpawnWhenDead = bAllowProjectileSpawnWhenDead;
return Task;
