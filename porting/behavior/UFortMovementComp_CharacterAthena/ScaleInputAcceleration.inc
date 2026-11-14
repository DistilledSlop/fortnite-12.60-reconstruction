// Original address: 0x1419efe30
// CONFIDENCE: high
FVector Result = Super::ScaleInputAcceleration(InputAcceleration);
if (Result.IsNearlyZero(0.0001f))
{
	return Result;
}

const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
if (!Player || !Player->IsSprinting() || !Player->IsLocallyControlled())
{
	return Result;
}

const FFortPawnStats* Stats = Player->PawnStatHandle.GetRow<FFortPawnStats>(TEXT("AFortPawn::GetPawnStats"));
const float StrafeMult = Stats ? FMath::Clamp(Stats->AccelerationStrafeMultiplierSprint, 0.0f, 1.0f) : 1.0f;
if (FMath::IsNearlyEqual(StrafeMult, 1.0f, 1e-8f))
{
	return Result;
}

const FVector RightVec = Player->GetRootComponent() ? Player->GetRootComponent()->GetRightVector() : Player->GetActorRightVector();
const float OrigSize = Result.Size();
const float RightDot = FVector::DotProduct(Result, RightVec);
FVector Adjusted = Result - RightVec * (RightDot * (1.0f - StrafeMult));
const float AdjSizeSq = Adjusted.SizeSquared();
if (AdjSizeSq > 1e-8f)
{
	Adjusted *= OrigSize * FMath::InvSqrt(AdjSizeSq);
}
return Adjusted;
