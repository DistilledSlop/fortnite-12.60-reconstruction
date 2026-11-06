// CONFIDENCE: high; captured native flow and typed dependency behavior.
const AFortAIPawn* Pawn = Defender;
if (!Pawn)
{
    return false;
}
FVector PawnLocation = Pawn->GetActorLocation();
return IsAnyEncounterGoalWithinDistanceOfPoint(PawnLocation, EncounterRelevantDistanceToDefender, true);
