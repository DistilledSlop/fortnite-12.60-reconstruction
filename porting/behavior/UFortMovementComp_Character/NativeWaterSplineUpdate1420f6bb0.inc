// Original address: 0x1420f6bb0
// CONFIDENCE: high
const float PrevImmersion = NativeRecoveredFloat_0x1d08;
const float PrevSurfaceDepth = NativeWaterSurfaceOffsetZ_1d10;
const FVector PrevSurfaceLocation = NativeRecoveredFVector_0x1d3c;

AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
AFortWaterBodyActor* WaterBody = Player ? Player->CurrentWaterBody : nullptr;

if (Player && WaterBody && !Player->IsInVehicle() && NativeHasSurfaceWater1420e5490())
{
	USceneComponent* RootComp = Player->GetRootComponent();
	UCapsuleComponent* Capsule = Player->GetCapsuleComponent();
	const FVector ActorLocation = RootComp ? RootComp->GetComponentLocation() : FVector::ZeroVector;
	const float HalfHeight = Capsule ? Capsule->GetScaledCapsuleHalfHeight() : 88.0f;

	const uint8 BodyTypeByte = static_cast<uint8>(WaterBody->GetWaterBodyType());
	if (NativeWaterSplineKey_1d04 <= 0.0f
		|| (BodyTypeByte != 0 && BodyTypeByte != 1))
	{
		NativeWaterSplineKey_1d04 = WaterBody->FindInputKeyClosestToWorldLocation(ActorLocation);
	}

	const FVector QueryPoint(ActorLocation.X, ActorLocation.Y, ActorLocation.Z - (HalfHeight + 0.1f));
	const EWaterBodyQueryFlags QueryFlags =
		EWaterBodyQueryFlags::ComputeLocation |
		EWaterBodyQueryFlags::ComputeNormal |
		EWaterBodyQueryFlags::ComputeDepth |
		EWaterBodyQueryFlags::ComputeImmersionDepth |
		EWaterBodyQueryFlags::IncludeWaves;
	const FWaterBodyQueryResult QueryResult =
		WaterBody->QueryWaterInfoClosestToWorldLocation(QueryPoint, QueryFlags, TOptional<float>(NativeWaterSplineKey_1d04));

	const float Immersion = QueryResult.GetImmersionDepth();
	NativeRecoveredFloat_0x1d08 = Immersion;
	NativeWaterSurfaceOffsetZ_1d10 = QueryResult.IsInExclusionVolume() ? 0.0f : QueryResult.GetWaterSurfaceDepth();
	NativeWaterCurrentVelocity_1d18 = WaterBody->GetWaterVelocityVectorAtSplineInputKey(NativeWaterSplineKey_1d04);

	const float MinDepth = (Player->bIsDBNO
		? SurfaceSwimmingWaterControlParams.DBNOSwimImmersionDepth
		: SurfaceSwimmingWaterControlParams.TargetImmersionDepth).GetValueAtLevel(0.0f);
	const float DepthForMax = SurfaceSwimmingWaterControlParams.WaterVelocityDepthForMax.GetValueAtLevel(0.0f);
	const float MinMult = SurfaceSwimmingWaterControlParams.WaterVelocityMinMultiplier.GetValueAtLevel(0.0f);

	float DepthAlpha = 0.0f;
	if (FMath::Abs(DepthForMax - MinDepth) > 1e-8f)
	{
		DepthAlpha = FMath::Clamp((NativeWaterSurfaceOffsetZ_1d10 - MinDepth) / (DepthForMax - MinDepth), 0.0f, 1.0f);
	}
	else
	{
		DepthAlpha = (NativeWaterSurfaceOffsetZ_1d10 < DepthForMax) ? 0.0f : 1.0f;
	}

	const float DepthScale = (1.0f - MinMult) * DepthAlpha + MinMult;
	NativeWaterVelocityDepthScale_1d14 = DepthScale;
	const FVector ScaledCurrentVel = NativeWaterCurrentVelocity_1d18 * DepthScale;
	NativeRecoveredFVector_0x1d24 = ScaledCurrentVel;

	FVector WaterDirVel(ScaledCurrentVel.X, ScaledCurrentVel.Y, 0.0f);
	if (FMath::Abs(Velocity.X) > 0.1f || FMath::Abs(Velocity.Y) > 0.1f)
	{
		const FVector VelDir2D = FVector(Velocity.X, Velocity.Y, 0.0f).GetSafeNormal();
		const float ProjSpeed = FVector::DotProduct(VelDir2D, FVector(ScaledCurrentVel.X, ScaledCurrentVel.Y, 0.0f));
		WaterDirVel = VelDir2D * ProjSpeed;
	}

	const FVector SurfLoc = QueryResult.GetWaterSurfaceLocation();
	NativeRecoveredFVector_0x1d3c = SurfLoc;
	NativeRecoveredFVector_0x1d30 = FVector(Velocity.X - WaterDirVel.X, Velocity.Y - WaterDirVel.Y, -WaterDirVel.Z);
	NativeRecoveredFVector_0x1d48 = QueryResult.IsInExclusionVolume() ? FVector::UpVector : QueryResult.GetWaterSurfaceNormal();

	const float BottomZ = QueryPoint.Z;
	const float TopZ = ActorLocation.Z + HalfHeight;
	const float Denom = TopZ - BottomZ;
	NativeWaterNormDepth_1d0c = (Denom > 1e-6f) ? FMath::Clamp((SurfLoc.Z - BottomZ) / Denom, 0.0f, 1.0f) : 0.0f;

	if (PrevImmersion <= 0.0f && Immersion > 0.0f)
	{
		WaterBody->OnPlayerEnteredWaterBody(Player, true);
	}
	else if (PrevImmersion > 0.0f && Immersion < 0.0f)
	{
		WaterBody->OnPlayerExitedWaterBody(Player, true);
	}

	if (Immersion > 0.0f && Player->IsSprinting())
	{
		if (Player->bIsWaterSprintBoostPending && !Player->bIsWaterSprintBoost)
		{
			const FVector Rel2D(NativeRecoveredFVector_0x1d30.X, NativeRecoveredFVector_0x1d30.Y, 0.0f);
			const float Rel2DSq = Rel2D.SizeSquared();
			const float TriggerVel = WaterSprintBoostParams.TriggerAtVelocity.GetValueAtLevel(0.0f);
			const FVector Forward = Player->GetRootComponent() ? Player->GetRootComponent()->GetForwardVector() : Player->GetActorForwardVector();
			const FVector VelDir = Velocity.GetSafeNormal();
			const float AlignDot = FVector::DotProduct(Forward, VelDir);

			if (Rel2DSq < TriggerVel * TriggerVel || AlignDot <= 0.3f)
			{
				const float PreventVel = WaterSprintBoostParams.PreventAtVelocity.GetValueAtLevel(0.0f);
				if (Rel2DSq <= PreventVel * PreventVel)
				{
					Player->bIsWaterSprintBoostPending = false;
				}
			}
			else if (Immersion <= WaterSprintBoostParams.TriggerAboveDepth.GetValueAtLevel(0.0f))
			{
				if (Player->GetLocalRole() != ROLE_SimulatedProxy)
				{
					Player->bIsWaterSprintBoost = true;
				}
				Player->NativeWaterSprintBoostTimeRemaining12d4 = WaterSprintBoostParams.Duration.GetValueAtLevel(0.0f);
			}
		}
		else if (!Player->bIsWaterSprintBoost && !Player->bIsSprintJump && !Player->bIsDBNO)
		{
			const float ReqDepth = WaterSprintBoostParams.RequiredDepth.GetValueAtLevel(0.0f);
			const float PreventVel = WaterSprintBoostParams.PreventAtVelocity.GetValueAtLevel(0.0f);
			const float Rel2DSq = FVector2D(NativeRecoveredFVector_0x1d30.X, NativeRecoveredFVector_0x1d30.Y).SizeSquared();
			if (Immersion >= ReqDepth && Rel2DSq > PreventVel * PreventVel)
			{
				Player->bIsWaterSprintBoostPending = true;
			}
		}
	}
}
else
{
	NativeWaterSplineKey_1d04 = -1.0f;
	NativeRecoveredFloat_0x1d08 = 0.0f;
	NativeWaterNormDepth_1d0c = 0.0f;
	NativeWaterSurfaceOffsetZ_1d10 = 0.0f;
	NativeWaterVelocityDepthScale_1d14 = 1.0f;
	NativeWaterCurrentVelocity_1d18 = FVector::ZeroVector;
	NativeRecoveredFVector_0x1d24 = FVector::ZeroVector;
	NativeRecoveredFVector_0x1d30 = FVector::ZeroVector;
	NativeRecoveredFVector_0x1d3c = FVector::ZeroVector;
	NativeRecoveredFVector_0x1d48 = FVector::ZeroVector;

	if (Player && PrevImmersion > 0.0f)
	{
		if (AFortWaterBodyActor* ExitBody = Player->CurrentWaterBody)
		{
			ExitBody->OnPlayerExitedWaterBody(Player, true);
		}
	}
}
