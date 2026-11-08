// Behaviour snippet: UFortSettingsContext::GamepadBuildingMultiplierValueChanged (body only).
// Original address: 0x144d00690 (thunk 0x1452cd5a0)
// Offset evidence: record = NativeGetSettingsRecord144d03d50() (ported helper). record+0x2f4 = GamepadBuildingMultiplier (Float). record+0x87e = bDirty (bool). UCOMISS + JZ: the write runs only when NewValue is ordered and unequal to the stored value (NaN takes the skip path).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be complete for the settings writes");
if (UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50())
{
	if (NewValue < SettingsRecord->GamepadBuildingMultiplier || NewValue > SettingsRecord->GamepadBuildingMultiplier)
	{
		SettingsRecord->bDirty = true;
		SettingsRecord->GamepadBuildingMultiplier = NewValue;
	}
}
