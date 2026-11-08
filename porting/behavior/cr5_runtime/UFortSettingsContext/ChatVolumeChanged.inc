// Behaviour snippet: UFortSettingsContext::ChatVolumeChanged (body only).
// Original address: 0x144cfc250 (thunk 0x1452cd390)
// Offset evidence: record = NativeGetSettingsRecord144d03d50() (ported helper); the record is passed to UFortClientSettingsRecord::SetChatVolume(float) at 0x141e3aa60 with NewValue in XMM1.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be complete to call its setter");
if (UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50())
{
	SettingsRecord->SetChatVolume(NewValue);
}
