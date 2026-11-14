// CONFIDENCE: high
// Original 0x1420e4ab0.
if (!CharacterOwner || MovementMode != MOVE_Custom ||
    (CustomMovementMode != 4 && CustomMovementMode != 5)) return false;
const FVector& InitialAcceleration = NativeAirAcceleration();
// COMISS/JA tests each component: unordered components do not exceed the tolerance.
const bool bHasAcceleration = FMath::Abs(InitialAcceleration.X) > 0.0001f
    || FMath::Abs(InitialAcceleration.Y) > 0.0001f
    || FMath::Abs(InitialAcceleration.Z) > 0.0001f;
if (!bHasAcceleration) return false;
const FVector Forward = NativeAirForward_1420e2860();
const FVector DirectionAcceleration = NativeAirAcceleration();
const float SquareSum = DirectionAcceleration.SizeSquared();
// UCOMISS/JZ skips normalization for both unit length and an unordered sum.
const FVector Direction = SquareSum == 1.0f || FMath::IsNaN(SquareSum)
    ? DirectionAcceleration : DirectionAcceleration.GetSafeNormal(1.e-8f);
const AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(CharacterOwner);
if (Pawn && Pawn->CurrentSkyTube)
{
    const AFortSkyTube* Tube = Pawn->CurrentSkyTube;
    return FVector::DotProduct(Direction, Forward) > Tube->NativeActiveForwardSteeringThreshold_0350;
}
if (-0.1f <= Direction.Z && (Direction.Z <= 0.1f || !NativeVortexOrSkyTube_1420d9730())) return false;
return FVector::DotProduct(Direction, Forward) > 0.4f;
