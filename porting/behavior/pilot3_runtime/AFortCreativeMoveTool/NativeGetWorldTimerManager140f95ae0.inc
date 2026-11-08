// Helper 0x140f95ae0: UWorld::GetTimerManager() of the receiver's world, returned as a pointer.
// Evidence: vtable slot 0x150 on RCX (shipping UObject vtable, GetWorld: the editor-only virtuals before it are compiled out);
//   result used as UWorld: lookup_offset UWorld 0x188 = OwningGameInstance (reflected); [GameInstance+0xc0] = FTimerManager*;
//   null game instance falls back to the world's own [UWorld+0x320] timer manager = UWorld::GetTimerManager().
// Callers (AAIHotSpot, AAthenaGoosePickup, ABuildingGameplayActor...) pass differently typed UObjects and use the result as FTimerManager*.
// CONFIDENCE: MEDIUM
UWorld* ContextWorld = Context->GetWorld();
return &ContextWorld->GetTimerManager();
