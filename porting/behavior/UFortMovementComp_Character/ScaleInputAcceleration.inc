// Original address: 0x1420ecb40
// CONFIDENCE: high
FVector Result = Super::ScaleInputAcceleration(InputAcceleration);

AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
if (!Player)
{
	return Result;
}

const float MaxSpeed = GetMaxSpeed();
const float SpeedRatio = (FMath::Abs(MaxSpeed) > 1e-8f) ? (Velocity.Size2D() / MaxSpeed) : 0.0f;

USceneComponent* Root = Player->GetRootComponent();
const FVector RightVec = Root ? Root->GetRightVector() : FVector::RightVector;
const FVector ForwardVec = Root ? Root->GetForwardVector() : FVector::ForwardVector;
const FVector UpVec = Root ? Root->GetUpVector() : FVector::UpVector;

const float RightProj = FVector::DotProduct(Result, RightVec);
const float ForwardProj = FVector::DotProduct(Result, ForwardVec);
const float UpProj = FVector::DotProduct(Result, UpVec);

FVector RightComp = RightVec * RightProj;
FVector ForwardComp = ForwardVec * ForwardProj;
const FVector UpComp = UpVec * UpProj;

const FVector AccelDir = Result.GetSafeNormal();

if (ForwardProj < 0.0f && VelocityBasedBackupCurve)
{
	const float MaxBackupScale = FMath::Clamp(VelocityBasedBackupCurve->GetFloatValue(SpeedRatio), 0.0f, 1.0f);
	const float AbsFwd = FMath::Abs(FVector::DotProduct(AccelDir, ForwardVec));
	if (AbsFwd > MaxBackupScale && AbsFwd > 1e-8f)
	{
		ForwardComp *= (MaxBackupScale / AbsFwd);
	}
}

if (VelocityBasedStrafeCurve)
{
	const float MaxStrafeScale = FMath::Clamp(VelocityBasedStrafeCurve->GetFloatValue(SpeedRatio), 0.0f, 1.0f);
	const float AbsRight = FMath::Abs(FVector::DotProduct(AccelDir, RightVec));
	if (AbsRight > MaxStrafeScale && AbsRight > 1e-8f)
	{
		RightComp *= (MaxStrafeScale / AbsRight);
	}
}

Result = UpComp + ForwardComp + RightComp;

if (MovementMode == MOVE_Custom && CustomMovementMode == 4 && NativeHasSkyTube_1420e5300())
{
	if (AFortSkyTube* Tube = Player->CurrentSkyTube)
	{
		const FVector ForwardAir = NativeAirForward_1420e2860();
		const FVector WorldLoc = UpdatedComponent ? UpdatedComponent->GetComponentLocation() : Player->GetActorLocation();
		const FVector AlignedAxis = Tube->NativeAlignVectorWithSplineAtLocation1427016d0(WorldLoc, ForwardAir);
		if (Tube->NativeShouldReduceSteering142707a50(AlignedAxis, ForwardAir, Result))
		{
			const float Proj = FVector::DotProduct(Result, AlignedAxis);
			Result -= AlignedAxis * Proj;
		}
	}
}

return Result;
