// Behaviour snippet: UFortSettingsContext::AnonymousCharacterModeChanged (body only; spliced into the generated member function).
// Original address: 0x144cf6c80 (via exec wrapper 0x1452cd1e0)
// Source: local-model fn runner; static checks only, not compiled or runtime verified.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord != nullptr)
{
	if (SettingsRecord->bAnonymousCharacterMode != bInChecked)
	{
		SettingsRecord->bAnonymousCharacterMode = bInChecked;
		SettingsRecord->bDirty = true;
	}
}
