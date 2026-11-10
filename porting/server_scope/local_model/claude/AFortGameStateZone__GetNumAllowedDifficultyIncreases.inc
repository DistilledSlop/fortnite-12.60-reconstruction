// CONFIDENCE: MEDIUM. Exec thunk 0x14292aef0 returns the int from the native lookup 0x141e9dc30, which reads the row named "AFortGameStateZone::GetNumDifficultyIncreaseTiers" from the difficulty DataTable row handle at this+0xb68 (FDataTableRowHandle::GetRow, 0x141e88ca0). Row field +0x68 is the tier count; a missing row or table returns 0.
// Original address: 0x14292aef0 (exec). Native: 0x141e9dc30 (deps AFortGameStateZone__GetDifficultyIncreaseTiersNative).
static const FName RowName(TEXT("AFortGameStateZone::GetNumDifficultyIncreaseTiers"));
const FFortDifficultyRow* Row = DifficultyRowHandle.GetRow(RowName);
if (Row == nullptr)
{
	return 0;
}
return Row->NumDifficultyIncreases;
