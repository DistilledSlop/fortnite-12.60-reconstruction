// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4bdc0. Server RPC receiver: validate, then implementation.
// Validate slot 0xf10 resolves to Stub_ReturnTrue (0x1406801d0), so the validate always passes.
if (!ServerModifyStat_Validate(StatName, Amount, ModType, bForceStatSave))
{
    RPC_ValidateFailed(TEXT("ServerModifyStat_Validate"));
    return;
}
ServerModifyStat_Implementation(StatName, Amount, ModType, bForceStatSave);   // slot 0xf18 -> 0x1423abba0
