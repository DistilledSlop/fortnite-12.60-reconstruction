// Original address: 0x1420da7a0
// CONFIDENCE: high
if (NativeZeroAccelFlag_1dff != 0)
{
	return FVector::ZeroVector;
}

FVector Result = InputAcceleration;
if (MovementMode == MOVE_Custom)
{
	if (CustomMovementMode == 4)
	{
		if (Result.Z > 0.0f && !NativeVortexOrSkyTube_1420d9730())
		{
			Result.Z = 0.0f;
		}
		return Result;
	}
	if (CustomMovementMode == 3)
	{
		if (Result.Z > 0.0f && !NativeHasSkyTube_1420e5300())
		{
			Result.Z = 0.0f;
		}
		return Result;
	}
	if (CustomMovementMode == 8 || CustomMovementMode == 10 || CustomMovementMode == 12)
	{
		Result.Z = 0.0f;
		return Result;
	}
}

return Super::ConstrainInputAcceleration(Result);
