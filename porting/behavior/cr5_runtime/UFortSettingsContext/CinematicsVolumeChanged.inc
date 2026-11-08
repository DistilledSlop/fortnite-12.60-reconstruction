// Behaviour snippet: UFortSettingsContext::CinematicsVolumeChanged (body only).
// Original address: 0x144cfc450 (thunk 0x1452cd410)
// Offset evidence: record = NativeGetSettingsRecord144d03d50() (ported helper); the record is passed to UFortClientSettingsRecord::SetCinematicsVolume(float) at 0x141e3aa80 with NewValue in XMM1.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be complete to call its setter");
if (UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50())
{
	SettingsRecord->SetCinematicsVolume(NewValue);
}
