// CONFIDENCE: LOW. Original exec 0x142a5dd50 (RPC thunk, parameters Volume (object) then bRestricted (bool), in thunk read order).
// Validate is virtual slot 0x2868 -> Stub_ReturnTrue (0x1406801d0), so no validation runs (STUB).
// Implementation is virtual slot 0x2870 -> 0x141a06860, inlined below. The decompiled implementation's argument labels are swapped relative to the thunk; this follows the thunk order.
// FUN_141c74820 (Athena state for the plot, LOW), FUN_141c94380 (appends {Volume, bRestricted} to the plot restriction list at +0x558, read in full).
if (UObject* Plot = GetAthenaStateForPlot(this))
{
	Plot->UGCRestrictedVolumes.Add(FUGCVolumeRestriction{ Volume, bRestricted }); // FUN_1406a9dc0 on Plot+0x558
}
