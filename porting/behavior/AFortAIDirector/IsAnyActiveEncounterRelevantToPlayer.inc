// CONFIDENCE: high; captured native flow and typed dependency behavior.
const AFortPlayerPawn* Pawn = Player;
if (!Pawn)
{
    return false;
}
FVector PawnLocation = Pawn->GetActorLocation();
return IsAnyEncounterGoalWithinDistanceOfPoint(PawnLocation, EncounterRelevantDistanceToPlayer, true);
