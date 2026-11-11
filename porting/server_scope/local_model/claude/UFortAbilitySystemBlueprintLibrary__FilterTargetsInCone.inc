// CONFIDENCE: MEDIUM. Returns the candidates that lie inside a cone running from ConeStart to ConeEnd. The cone apex is ConeStart, the cone length is |ConeEnd - ConeStart|, and the half angle is asin(ConeRadius / sqrt(ConeRadius^2 + Length^2)). When bShouldCheckLos is set, a candidate is dropped if a blocking line trace from the cone origin hits something that does not carry an excluded tag.
// Original addresses: exec 0x142780de0; body helper 0x141df2a70.
TArray<UObject*> ReturnValue;
UWorld* World = GEngine->GetWorldFromContextObject(SourceActor, EGetWorldErrorMode::LogAndReturnNull);
if (!World)
{
	return ReturnValue;
}

const float ConeLengthSq = (ConeEnd - ConeStart).SizeSquared();
const FVector ConeAxis = (ConeEnd - ConeStart).GetSafeNormal();
const float HalfAngleSin = ConeRadius / FMath::Sqrt(ConeRadius * ConeRadius + ConeLengthSq);
const float CosHalfAngle = FMath::Cos(FMath::Asin(HalfAngleSin));

for (UObject* Candidate : ActorsToCheck)
{
	AActor* Target = Cast<AActor>(Candidate);
	if (!Target)
	{
		continue;
	}

	const FVector TargetLocation = Target->GetActorLocation();
	const FVector Offset = TargetLocation - ConeStart;
	if (Offset.SizeSquared() > ConeLengthSq)
	{
		continue;   // beyond the far end of the cone
	}
	if (FVector::DotProduct(ConeAxis, Offset.GetSafeNormal()) < CosHalfAngle)
	{
		continue;   // outside the angular extent
	}

	if (bShouldCheckLos)
	{
		TArray<FHitResult> Hits;
		World->LineTraceMultiByChannel(Hits, ConeStart, TargetLocation, ECC_Visibility, FCollisionQueryParams::DefaultQueryParam);   // trace channel from global 0x14812bab8 (ECC_Visibility assumed)
		bool bObstructed = false;
		for (const FHitResult& Hit : Hits)
		{
			if (!Hit.bBlockingHit)
			{
				continue;
			}
			AActor* HitActor = Hit.GetActor();
			if (HitActor && HasAnyExcludedTag(HitActor, ExcludeFromObstructionTags))   // helper 0x143e98d10
			{
				continue;
			}
			bObstructed = true;
			break;
		}
		if (bObstructed)
		{
			continue;
		}
	}

	ReturnValue.Add(Target);
}
return ReturnValue;
