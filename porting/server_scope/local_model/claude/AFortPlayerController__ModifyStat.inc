// CONFIDENCE: MEDIUM - the parameter block is fully mapped; the Blueprint event FName (DAT_1481af1c0) is not resolved to a string.
// Original address: 0x142a48a40 (exec thunk). Forward chain: 0x142395320 (tail) -> 0x142a3f020 (ProcessEvent wrapper).
// Declaration order (inventory list reversed; the thunk reads StatName, Amount, ModType, bForceStatSave in that order):
//   StatName (FName), Amount (int32), ModType (enum, 1 byte), bForceStatSave (bool).
// The parameter block layout matches the wrapper: Name at 0, int32 at 8, byte at 12, bool at 13.
struct FModifyStatParams
{
	FName StatName;
	int32 Amount;
	uint8 ModType;
	bool bForceStatSave;
};
FModifyStatParams Params;
Params.StatName = StatName;
Params.Amount = Amount;
Params.ModType = (uint8)ModType;
Params.bForceStatSave = bForceStatSave;
UFunction* Event = FindFunctionChecked(FName_0x1481af1c0);
ProcessEvent(Event, &Params);
