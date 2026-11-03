// CONFIDENCE: high; captured native flow and typed dependency behavior.
const AFortPlayerPawn* Pawn = Player;
if (!Pawn)
{
    return false;
}
return IsAnyEncounterGoalWithinDistanceOfPoint(Pawn->GetActorLocation(), EncounterRelevantDistanceToPlayer, true);
