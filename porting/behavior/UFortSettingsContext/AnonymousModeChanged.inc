// Behaviour snippet: UFortSettingsContext::AnonymousModeChanged (body only; spliced into the generated member function).
// Original address: 0x144cf6cb0 (via exec wrapper 0x1452cd270)
// Source: local-model fn runner; static checks only, not compiled or runtime verified.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord != nullptr)
{
	if (SettingsRecord->bAnonymousMode != bInChecked)
	{
		SettingsRecord->bAnonymousMode = bInChecked;
		SettingsRecord->bDirty = true;
	}
}
