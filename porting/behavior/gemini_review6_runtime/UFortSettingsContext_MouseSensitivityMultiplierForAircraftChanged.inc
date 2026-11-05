// Behaviour snippet: UFortSettingsContext::MouseSensitivityMultiplierForAircraftChanged (body only; spliced into the generated member function).
// Original address: 0x144d14cb0 (via exec wrapper 0x1452cf910)
// Source: local-model fn runner; static checks only, not compiled or runtime verified.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord != nullptr && FMath::Abs(NewValue - SettingsRecord->MouseSensitivityMultiplierForAircraftValue) > 1e-08f)
{
	SettingsRecord->MouseSensitivityMultiplierForAircraftValue = NewValue;
	SettingsRecord->bDirty = true;
}
