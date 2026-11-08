// Helper 0x143e8a9c0 (host UFortMatchmakingKnobsModal). Asm: 143e8a9c0..143e8aa56.
// Arguments: RCX = TArray of 8-byte pairs (two int32, FIntPoint layout), RDX = second TArray of the same type.
// Caller 0x144dbb010 passes two local TArrays (RSP+0x20 and RSP+0x40).
// Evidence: Num() of both arrays must match ([+8] int32, MOVSXD), else false. For each element of the left array,
// a right-array element with equal first and second int32 must exist, else false. Empty left array -> true.
// Every instruction maps to a statement below.
// CONFIDENCE: HIGH
if (LeftPairs.Num() != RightPairs.Num())
{
	return false;
}
for (const FIntPoint& LeftPair : LeftPairs)
{
	bool bPairFound = false;
	for (const FIntPoint& RightPair : RightPairs)
	{
		if (RightPair.X == LeftPair.X && RightPair.Y == LeftPair.Y)
		{
			bPairFound = true;
			break;
		}
	}
	if (!bPairFound)
	{
		return false;
	}
}
return true;
