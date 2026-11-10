// dep AFortGameStateZone__TrySetDifficultyIncreaseRewardTier original 0x141eaec30
// CONFIDENCE: MEDIUM. Original 0x141eaec30. The zone-difficulty row (ZoneDifficultyInfoRow, +0xb68) caps the tier. Static init of the context string is omitted.
int32 NumDifficultyIncreaseTiers = 0;
const FZoneDifficultyRow* Row = ZoneDifficultyInfoRow.GetRow(TEXT("AFortGameStateZone::GetNumDifficultyIncreaseTiers"));   // 0x141e88ca0
if (Row != nullptr)
{
	NumDifficultyIncreaseTiers = Row->NumDifficultyIncreaseTiers;   // row+0x68
}
if (NumDifficultyIncreaseTiers < InTier)
{
	return false;
}
DifficultyIncreaseRewardTier = InTier;   // +0xcd0
return true;
