// CONFIDENCE: HIGH - RPC exec thunk.
// Original exec 0x142a583d0. Validate: vtable +0x26e0 = folded 0x140f6b550 (DestroyCreativePlot_Validate, ported).
// Implementation: vtable +0x26e8 = 0x1419ca250 (separate RPC body, not ported in this chunk; see deps/).
if (!DestroyCreativePlot_Validate(IslandId))
{
    RPC_ValidateFailed(TEXT("DestroyCreativePlot_Validate"));
}
else
{
    DestroyCreativePlot_Implementation(IslandId);
}
