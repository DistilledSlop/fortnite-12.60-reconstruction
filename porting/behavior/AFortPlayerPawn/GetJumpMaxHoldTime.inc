// Original address: 0x14231ed20
// CONFIDENCE: high
if (const UFortMovementComp_Character* FortMove = Cast<UFortMovementComp_Character>(GetCharacterMovement()))
{
	if (FortMove->MovementMode == MOVE_Custom && FortMove->CustomMovementMode == EFortCustomMovement::SurfaceSwimming)
	{
		const FWaterJumpParams& Params = IsSprinting() ? FortMove->WaterJumpParamsSprint : FortMove->WaterJumpParams;
		return Params.JumpForceDuration.GetValueAtLevel(0.0f);
	}
}
if (bIsWaterJump)
{
	return 0.0f;
}
static const FFortPlayerPawnStats DefaultPlayerPawnStats;
const FFortPlayerPawnStats* Stats = PawnStatHandle.GetRow<FFortPlayerPawnStats>(TEXT("AFortPawn::GetPawnStats"));
if (!Stats)
{
	Stats = &DefaultPlayerPawnStats;
}
if (!MovementSet)
{
	return Stats->MaxJumpTime;
}
if (AbilitySystemComponent)
{
	static FProperty* JumpHeightProp = FindFieldChecked<FProperty>(
		UFortMovementSet::StaticClass(), FName(TEXT("JumpHeight")));
	static const FGameplayEffectAttributeCaptureDefinition CaptureDef(
		FGameplayAttribute(JumpHeightProp), EGameplayEffectAttributeCaptureSource::Source, true);
	FGameplayEffectAttributeCaptureSpec CaptureSpec(CaptureDef);
	AbilitySystemComponent->CaptureAttributeForGameplayEffect(CaptureSpec);
	FAggregatorEvaluateParameters EvalParams;
	float EvaluatedMaxJumpTime = Stats->MaxJumpTime;
	if (CaptureSpec.AttemptCalculateAttributeMagnitudeWithBase(EvalParams, Stats->MaxJumpTime, EvaluatedMaxJumpTime))
	{
		return EvaluatedMaxJumpTime;
	}
}
return Stats->MaxJumpTime;
