// Native 0x141e9dc30. Lazily builds the static row name "AFortGameStateZone::GetNumDifficultyIncreaseTiers", then reads it from the difficulty row handle at this+0xb68 (FDataTableRowHandle::GetRow, 0x141e88ca0). Missing table or row returns 0; a present row returns its field at +0x68.
static const FName RowName(TEXT("AFortGameStateZone::GetNumDifficultyIncreaseTiers"));
const FFortDifficultyRow* Row = DifficultyRowHandle.GetRow(RowName);
if (Row == nullptr)
{
	return 0;
}
return Row->NumDifficultyIncreases;
