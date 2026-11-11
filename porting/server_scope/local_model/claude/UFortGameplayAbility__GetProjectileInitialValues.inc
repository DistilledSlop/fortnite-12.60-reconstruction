// CONFIDENCE: LOW. Original exec 0x1428f86b0; core 0x1415fc430.
// Bool inputs map to core params as: 6 = bApplyInaccuracy (rand jitter), 7 = bApplyClampedMaxYawAngleToFire, 8 = bApplyLeading (target lead).
// The reflection order lists them as Leading, ClampedMaxYaw, Inaccuracy; semantics above come from what each gate calls.
OutProjectileSpeed = 0.f;
OutProjectileGravity = 0.f;
OutSpawnDirection = FRotator::ZeroRotator;

HomingTarget = GetHomingTargetActor();
if (!IsValid(HomingTarget))
{
	// Native core only writes outputs when a homing target exists; the outputs keep their zero defaults otherwise.
	return;
}

const FVector FireLocation = GetFireLocation();
const float ProjectileSpeed = GetProjectileSpeed(0);
FVector LaunchVelocity = FVector::ZeroVector;

if (ProjectileLaunchAngleDegrees == 0.f)
{
	// Level shot: aim direction (with the stored aim flag and inaccuracy) scaled by projectile speed, no gravity.
	const FVector AimDirection = GetProjectileAimDirection(bUseAIFireLocationAndRotation, bApplyInaccuracy);
	LaunchVelocity = AimDirection.GetSafeNormal() * ProjectileSpeed;
	OutProjectileGravity = 0.f;
}
else
{
	// Arc shot: aim at the target, optionally lead it by time-to-target, jitter the aim, then solve the ballistic launch.
	FVector AimLocation;
	GetHomingAimPoint(FireLocation, HomingTarget, AimLocation);
	if (bApplyLeading && bUseAIFireLocationAndRotation && IsValid(ProjectileMovementComponent))
	{
		const float TimeToTarget = GetProjectileTimeToTarget(FireLocation, AimLocation);
		AimLocation = PredictTargetLocation(HomingTarget, TimeToTarget);
		AimLocation = ClampAimToProjectileRange(FireLocation, AimLocation);
	}
	if (bApplyInaccuracy)
	{
		AimLocation = ApplyProjectileInaccuracy(FireLocation, AimLocation);
	}
	LaunchVelocity = SolveProjectileLaunchVelocity(FireLocation, AimLocation, ProjectileSpeed, ProjectileLaunchAngleDegrees, OutProjectileGravity);
}

OutSpawnDirection = LaunchVelocity.Rotation();
if (bApplyClampedMaxYawAngleToFire && bUseMaxYawAngleToFire)
{
	const AFortPawn* Pawn = GetOwningFortPawn();
	if (IsValid(Pawn))
	{
		OutSpawnDirection = ClampAimYawToMaxFireAngle(OutSpawnDirection, Pawn->GetActorRotation());
	}
}
OutProjectileSpeed = LaunchVelocity.Size();
