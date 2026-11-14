// Original address: 0x1420ee220
// CONFIDENCE: high
if (AFortAIPawn* AIPawn = Cast<AFortAIPawn>(CharacterOwner))
{
	if (UCapsuleComponent* Capsule = AIPawn->GetCapsuleComponent())
	{
		const float HalfHeight = Capsule->GetUnscaledCapsuleHalfHeight() * Capsule->GetComponentScale().Z;
		const float ScaleFactor = (HalfHeight > 1e-4f) ? FMath::Max(70.0f / HalfHeight, 1.0f) : 1.0f;
		NavMeshProjectionHeightScaleDown = ScaleFactor * 1.5f;
		NavMeshProjectionHeightScaleUp = ScaleFactor * 1.25f;
	}
	NavMeshProjectionTimer = (NavMeshProjectionInterval > 0.0f) ? (FMath::FRand() * 2.0f - 1.0f) * NavMeshProjectionInterval : 0.0f;
	CachedProjectedNavMeshHitResult.Reset(1.0f, false);
	CachedProjectedNavMeshHitResult.Location = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : FVector::ZeroVector;
	return;
}
Super::SetNavWalkingPhysics(bEnable);
