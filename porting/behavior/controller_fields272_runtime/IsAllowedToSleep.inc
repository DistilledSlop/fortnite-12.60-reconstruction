// CONFIDENCE: high
// The native low-byte return is true only when there is no goal and the pawn can sleep.
return GoalActor == nullptr && MyFortPawn != nullptr && MyFortPawn->bCanSleep;
