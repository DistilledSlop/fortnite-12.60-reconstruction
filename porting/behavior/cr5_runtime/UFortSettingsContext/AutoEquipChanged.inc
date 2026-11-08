// Behaviour snippet: UFortSettingsContext::AutoEquipChanged (body only).
// Original address: 0x144cf77b0 (thunk 0x1452cd300)
// Offset evidence: record = NativeGetSettingsRecord144d03d50() (ported helper). record+0x470 = bAutoEquipBetterItems (bool, byte-form write of bInChecked). record+0x87e = bDirty (bool, set to 1 on change).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be complete for the settings writes");
if (UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50())
{
	if (SettingsRecord->bAutoEquipBetterItems != bInChecked)
	{
		SettingsRecord->bAutoEquipBetterItems = bInChecked;
		SettingsRecord->bDirty = true;
	}
}
