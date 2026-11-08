// Behaviour snippet: UFortSettingsContext::DialogVolumeChanged (body only).
// Original address: 0x144cfda10 (thunk 0x1452cd490)
// Offset evidence: record = NativeGetSettingsRecord144d03d50() (ported helper); the record is passed to UFortClientSettingsRecord::SetDialogueVolume(float) at 0x141e3b240 with NewValue in XMM1.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be complete to call its setter");
if (UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50())
{
	SettingsRecord->SetDialogueVolume(NewValue);
}
