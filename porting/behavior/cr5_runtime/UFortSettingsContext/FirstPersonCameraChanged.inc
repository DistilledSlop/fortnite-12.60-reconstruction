// Behaviour snippet: UFortSettingsContext::FirstPersonCameraChanged (body only).
// Original address: 0x144cfff10 (thunk 0x1452cd510)
// Offset evidence: record = NativeGetSettingsRecord144d03d50() (ported helper). record+0x47b = bUseFirstPersonCamera (bool, byte-form write of bInChecked). record+0x87e = bDirty (bool, set to 1 on change).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be complete for the settings writes");
if (UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50())
{
	if (SettingsRecord->bUseFirstPersonCamera != bInChecked)
	{
		SettingsRecord->bUseFirstPersonCamera = bInChecked;
		SettingsRecord->bDirty = true;
	}
}
