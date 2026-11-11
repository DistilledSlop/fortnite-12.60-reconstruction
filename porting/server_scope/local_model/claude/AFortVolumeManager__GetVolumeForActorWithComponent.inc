// CONFIDENCE: MEDIUM Original addresses: thunk 0x142b3bc60; see evidence.json for callees.
// Signature: AFortVolume* GetVolumeForActorWithComponent(AActor* Actor, TSubclassOf<UActorComponent> ComponentType, TArray<UActorComponent*>& OutComponents, bool bUseActorBounds)
// Helper proposed: NativeFindVolumeWithComponent (work/claude/out/deps/AFortVolumeManager__NativeFindVolumeWithComponent.json).
return NativeFindVolumeWithComponent(Actor, ComponentType, OutComponents, bUseActorBounds);
