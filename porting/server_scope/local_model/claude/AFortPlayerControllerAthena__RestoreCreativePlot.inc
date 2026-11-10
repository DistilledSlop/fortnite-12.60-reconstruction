// CONFIDENCE: HIGH - RPC exec thunk.
// Original exec 0x142a5a1f0. Validate: vtable +0x26c0 = folded 0x140f6b550 (RestoreCreativePlot_Validate, ported).
// Implementation: vtable +0x26c8 = 0x1419ef2a0 (separate RPC body, not ported in this chunk).
if (!RestoreCreativePlot_Validate(IslandId))
{
    RPC_ValidateFailed(TEXT("RestoreCreativePlot_Validate"));
}
else
{
    RestoreCreativePlot_Implementation(IslandId);
}
