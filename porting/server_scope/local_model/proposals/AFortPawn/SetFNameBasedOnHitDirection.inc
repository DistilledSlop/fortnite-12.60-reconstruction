// Behaviour snippet: AFortPawn::SetFNameBasedOnHitDirection (body only; spliced into the generated member function).
// Original address: 0x142a1f9d0 (thunk) -> 0x14234c7b0 (body)
// Decompile source: captures/05fdcca221474417af8f8f03a70ff6d8/0x14234c7b0.c
// Inferred: Reads the RootComponent (AActor::RootComponent, +0x130) rotation quaternion (+0x1c0) and converts it to the actor forward vector (identity when RootComponent is null). Computes the signed angle (radians) between the forward vector and the normalized HitDirection, with the sign taken from the up axis. Degrees = angle * 57.295776 (= 180/PI). Classification (abs degrees): <=45 -> NameFront; >=120 -> NameLeft; 45..120 -> NameRight when signed deg > 0 else NameLeft. NameBack is never written (red herring in the original).
// Declared in generated header: no (UFUNCTION(BlueprintCallable) void SetFNameBasedOnHitDirection(const FVector& HitDirection, FName& NameResult, FName NameFront, FName NameBack, FName NameLeft, FName NameRight); emitted when the behaviour splice is on)
// CONFIDENCE: MEDIUM
{
	// Forward from the RootComponent rotation (the decompile reads the quat at RootComponent+0x1c0 and builds the rotation matrix; GetActorForwardVector() is the equivalent accessor).
	const FVector Forward = GetActorForwardVector();
	const FVector Up = GetActorUpVector();
	const FVector HitDir = HitDirection.GetSafeNormal();

	// Signed angle (radians) between Forward and HitDir; magnitude via acos, sign via the up-axis cross product.
	const float Dot = FMath::Clamp(Forward.DotProduct(HitDir), -1.f, 1.f);
	const float Angle = FMath::Acos(Dot);
	const float Sign = (FVector::CrossProduct(Forward, HitDir).DotProduct(Up) >= 0.f) ? 1.f : -1.f;
	const float SignedDeg = FMath::RadiansToDegrees(Angle) * Sign;
	const float AbsDeg = FMath::Abs(SignedDeg);

	if (AbsDeg <= 45.f)
	{
		NameResult = NameFront;
	}
	else if (AbsDeg >= 120.f)
	{
		NameResult = NameLeft;
	}
	else
	{
		NameResult = (SignedDeg > 0.f) ? NameRight : NameLeft;
	}
}
