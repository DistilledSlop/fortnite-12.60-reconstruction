// Behaviour snippet: UFortSettingsContext::GamepadEditModeMultiplierValueChanged (body only).
// Original address: 0x144d006d0 (thunk 0x1452cd620)
// Offset evidence: record = NativeGetSettingsRecord144d03d50() (ported helper). record+0x2f8 = GamepadEditModeMultiplier (Float). record+0x87e = bDirty (bool). UCOMISS + JZ: the write runs only when NewValue is ordered and unequal to the stored value (NaN takes the skip path).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be complete for the settings writes");
if (UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50())
{
	if (NewValue < SettingsRecord->GamepadEditModeMultiplier || NewValue > SettingsRecord->GamepadEditModeMultiplier)
	{
		SettingsRecord->bDirty = true;
		SettingsRecord->GamepadEditModeMultiplier = NewValue;
	}
}
