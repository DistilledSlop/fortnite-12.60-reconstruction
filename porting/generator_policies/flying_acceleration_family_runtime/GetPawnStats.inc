// CONFIDENCE: HIGH
// Original AFortPawn::GetPawnStats, 0x141a07b70.
if (const FFortPawnStats* Row = PawnStatHandle.GetRow<FFortPawnStats>(TEXT("AFortPawn::GetPawnStats")))
    return Row;
static FFortPawnStats DefaultPawnStats;
return &DefaultPawnStats;
