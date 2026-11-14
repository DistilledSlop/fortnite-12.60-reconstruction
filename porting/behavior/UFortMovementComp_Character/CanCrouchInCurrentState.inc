// Original address: 0x1420d9600
// CONFIDENCE: high
AFortPawn* FortPawn = Cast<AFortPawn>(CharacterOwner);
AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
if (Player && Player->bIsDBNO)
{
	return true;
}

if (MovementMode == MOVE_Custom && (CustomMovementMode == 1 || CustomMovementMode == 2))
{
	return false;
}

if (FortPawn && FortPawn->EmoteCount != 0)
{
	return false;
}

if (Player && Player->bLocalIsSkydiving)
{
	return false;
}

if (FortPawn && !FortPawn->NativeCanPerformAction14230c260(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x698)))
{
	return false;
}

if (MovementMode == MOVE_Custom && CustomMovementMode == 10)
{
	return false;
}

if (NativeRecoveredFloat_0x1d08 > 0.0f &&
	NativeRecoveredFloat_0x1d08 >= SurfaceSwimmingWaterControlParams.CancelCrouchImmersionDepth.GetValueAtLevel(0.0f))
{
	return false;
}

return Super::CanCrouchInCurrentState();
