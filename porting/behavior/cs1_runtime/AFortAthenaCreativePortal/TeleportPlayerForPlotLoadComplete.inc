// Original body address: 0x1418d6a80 (thunk 0x1427d4770)
// Offset evidence: this+0xa78 is the reflected InteractIconPlacementCapsule WeakObject (8 bytes). The TArray<uint8*> at this+0xa80 is an unreflected gap; it is passed by reference to NativeRemoveMatchingWeakPtrs1407ef240 together with the weak key of PlayerPawn.
// CONFIDENCE: HIGH
TWeakObjectPtr<UObject> PawnWeak;
PawnWeak = PlayerPawn;
TArray<uint8*>& Items = *reinterpret_cast<TArray<uint8*>*>(reinterpret_cast<uint8*>(this) + 0xa80);
NativeRemoveMatchingWeakPtrs1407ef240(Items, reinterpret_cast<const uint8*>(&PawnWeak));
