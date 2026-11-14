// CONFIDENCE: high
// Original 0x1420e4740. Boolean return is AL, not Ghidra's spurious upper bits.
if (!CharacterOwner || MovementMode != MOVE_Custom || CustomMovementMode != 3) return false;
const FVector& InitialAcceleration = NativeAirAcceleration();
// COMISS/JA tests each component: unordered components do not exceed the tolerance.
const bool bHasAcceleration = FMath::Abs(InitialAcceleration.X) > 0.0001f
    || FMath::Abs(InitialAcceleration.Y) > 0.0001f
    || FMath::Abs(InitialAcceleration.Z) > 0.0001f;
if (!bHasAcceleration || !NativeHasSkyTube_1420e5300()) return false;
const FVector Forward = (CharacterOwner->Controller
    ? CharacterOwner->GetControlRotation() : CharacterOwner->GetBaseAimRotation()).Vector();
const FVector DirectionAcceleration = NativeAirAcceleration();
const float SquareSum = DirectionAcceleration.SizeSquared();
// UCOMISS/JZ skips normalization for both unit length and an unordered sum.
const FVector Direction = SquareSum == 1.0f || FMath::IsNaN(SquareSum)
    ? DirectionAcceleration : DirectionAcceleration.GetSafeNormal(1.e-8f);
const AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(CharacterOwner);
if (!Pawn || !Pawn->CurrentSkyTube) return false;
const float Dot = FVector::DotProduct(Direction, Forward);
const AFortSkyTube* Tube = Pawn->CurrentSkyTube;
const float Threshold = Tube->NativeActiveForwardSteeringThreshold_0350;
return Dot > Threshold;
