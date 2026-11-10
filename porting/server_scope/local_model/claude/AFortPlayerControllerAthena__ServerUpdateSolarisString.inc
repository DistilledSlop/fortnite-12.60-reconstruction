// CONFIDENCE: MEDIUM. Original exec 0x142a5cea0 (RPC thunk, one FString parameter SolString).
// Validate is virtual slot 0x2618 -> Stub_ReturnTrue (0x1406801d0), so no validation runs (STUB).
// Implementation is virtual slot 0x2620 -> 0x1419f3790, inlined below.
// Unresolved: the flag at OnUpdateCreativeDescriptionTags+0x28 (Field_0x28); plot string field at +0x2f0 (Field_0x2f0); controller virtual slot 0x14d8 (creative volume).
if ((GetAthenaTeamSettings(true)->Field_0x9fc & 1) != 0)
{
	if (GetFortEngine()->Field_0xc9 != 0)
	{
		if (UFortRuntimeOptions::GetDefault()->Field_0xb1b != 0 && OnUpdateCreativeDescriptionTags.Field_0x28 != 0)
		{
			UObject* Volume = GetCreativeVolumeForController(); // virtual slot 0x14d8 on this
			UObject* Plot = GetCreativePlotForVolume(Volume);   // FUN_141c7c460
			if (Plot != nullptr)
			{
				Plot->SolarisString_0x2f0 = SolString;                      // FUN_140683960: FString copy assignment
				UObject* StateOwner = GetAthenaStateForPlot(Plot);          // FUN_141c74820 (LOW)
				if (StateOwner != nullptr)
				{
					NotifyPlotSolarisStringChanged(StateOwner, Plot);       // FUN_141c8cf90: only when StateOwner Role == ROLE_Authority
				}
			}
		}
	}
}
