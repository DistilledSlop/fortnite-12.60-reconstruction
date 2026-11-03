// CONFIDENCE: high; captured native flow and typed dependency behavior.
const AFortAIPawn* Pawn = Defender;
if (!Pawn)
{
    return false;
}
return IsAnyEncounterGoalWithinDistanceOfPoint(Pawn->GetActorLocation(), EncounterRelevantDistanceToDefender, true);
