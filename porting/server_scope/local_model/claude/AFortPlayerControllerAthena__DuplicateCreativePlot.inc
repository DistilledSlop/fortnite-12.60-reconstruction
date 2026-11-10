// CONFIDENCE: HIGH - RPC exec thunk.
// Original exec 0x142a58490. FFrame read order: IslandId, Locale, Title.
// Validate: vtable +0x26a8 = folded 0x140f6b550 (DuplicateCreativePlot_Validate, ported).
// Implementation: vtable +0x26b0 = 0x1419cdd40 (separate RPC body, not ported in this chunk).
if (!DuplicateCreativePlot_Validate(IslandId, Locale, Title))
{
    RPC_ValidateFailed(TEXT("DuplicateCreativePlot_Validate"));
}
else
{
    DuplicateCreativePlot_Implementation(IslandId, Locale, Title);
}
