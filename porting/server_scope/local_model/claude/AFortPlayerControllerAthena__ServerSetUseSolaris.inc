// CONFIDENCE: MEDIUM. Original exec 0x142a5c530 (RPC thunk, one bool parameter bShouldUse).
// Validate is virtual slot 0x2628 -> Stub_ReturnTrue (0x1406801d0), so no validation runs (STUB).
// Implementation is virtual slot 0x2630 -> 0x1419f27c0, inlined below.
// Unresolved: the flag at OnUpdateCreativeDescriptionTags+0x28 (Field_0x28); controller virtual slot 0x14d8 (0x1419d5f00, creative volume for controller).
if ((GetAthenaTeamSettings(true)->Field_0x9fc & 1) != 0)
{
	if (GetFortEngine()->Field_0xc9 != 0) // FUN_141ed2350 engine getter
	{
		if (UFortRuntimeOptions::GetDefault()->Field_0xb1b != 0 && OnUpdateCreativeDescriptionTags.Field_0x28 != 0)
		{
			UObject* Volume = GetCreativeVolumeForController(); // virtual slot 0x14d8 on this
			UObject* Plot = GetCreativePlotForVolume(Volume);   // FUN_141c7c460
			if (Plot != nullptr)
			{
				SetPlotUseSolaris(Plot, bShouldUse);            // FUN_1429d6a00: ProcessEvent with a bool argument (UFunction name constant unresolved)
			}
		}
	}
}
