// CONFIDENCE: HIGH - RPC exec thunk.
// Original exec 0x142a591b0. FFrame read order: PlotType, Locale, Title.
// Validate: vtable +0x2708 = 0x14097cd20 (MakeNewCreativePlot_Validate, ported).
// Implementation: vtable +0x2710 = 0x1419df280 (separate RPC body, not ported in this chunk).
if (!MakeNewCreativePlot_Validate(PlotType, Locale, Title))
{
    RPC_ValidateFailed(TEXT("MakeNewCreativePlot_Validate"));
}
else
{
    MakeNewCreativePlot_Implementation(PlotType, Locale, Title);
}
