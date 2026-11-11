// CONFIDENCE: MEDIUM. Original exec 0x1428f84c0; core 0x1415fa300.
// Owner pawn location plus the ability fire offset rotated into pawn space; zero when there is no owning pawn.
AFortPawn* OwningPawn = GetOwningFortPawn();
if (!IsValid(OwningPawn))
{
	return FVector::ZeroVector;
}

const FTransform PawnTransform = OwningPawn->GetActorTransform();
return PawnTransform.GetLocation() + PawnTransform.GetRotation().RotateVector(FireLocationOffset);
