// CONFIDENCE: HIGH - RPC exec thunk.
// Original exec 0x142a59310. FFrame read order: LinkCode, Locale, Title.
// Validate: vtable +0x26f8 = folded 0x140f6b550 (MakeNewCreativePlotFromLinkCode_Validate, ported).
// Implementation: vtable +0x2700 = 0x1419dec70 (separate RPC body, not ported in this chunk).
if (!MakeNewCreativePlotFromLinkCode_Validate(LinkCode, Locale, Title))
{
    RPC_ValidateFailed(TEXT("MakeNewCreativePlotFromLinkCode_Validate"));
}
else
{
    MakeNewCreativePlotFromLinkCode_Implementation(LinkCode, Locale, Title);
}
