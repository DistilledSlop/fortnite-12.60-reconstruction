// CONFIDENCE: LOW Returns the aim location for Instigator. The base point is the instigator cached aim vector (sub-object at +0x130, vector at +0x1d0), defaulting to a constant vector when the sub-object is absent. If the instigator has an aim component (+0x250), its GetAimLocation (vtable slot 0x6f8) refines the result.
// Original: exec 0x1428f82e0 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f82e0.c)
// The "nearest pawn" wording in the name is not borne out by the native body, which does not search pawns.
FVector AimLocation = FVector::ZeroVector;
if (Instigator != nullptr)
{
	AimLocation = GetInstigatorCachedAimBase(Instigator);
	if (UActorComponent* AimComponent = GetInstigatorAimComponent(Instigator))
	{
		AimComponent->GetAimLocation(AimLocation);
	}
}
return AimLocation;
