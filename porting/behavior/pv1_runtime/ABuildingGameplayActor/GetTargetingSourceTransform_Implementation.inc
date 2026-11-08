// Original body 0x141bdd320 (thunk 0x1427401b0) for ABuildingGameplayActor::GetTargetingSourceTransform_Implementation.
// Offset evidence: [RCX+0x130] is AActor::RootComponent (reflected); [Root+0x1c0] is the 0x30-byte ComponentToWorld FTransform (raw, engine USceneComponent); fallback copies FTransform::Identity at 0x1482184f0.
// CONFIDENCE: HIGH
if (RootComponent != nullptr)
{
	const uint8* RootBytes = reinterpret_cast<const uint8*>(RootComponent);
	return *reinterpret_cast<const FTransform*>(RootBytes + 0x1c0);
}
return FTransform::Identity;
