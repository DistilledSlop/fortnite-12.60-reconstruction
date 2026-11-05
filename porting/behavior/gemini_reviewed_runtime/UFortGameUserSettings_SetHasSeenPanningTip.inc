// Behaviour snippet: UFortGameUserSettings::SetHasSeenPanningTip (body only; spliced into the generated member function).
// Original address: 0x141eafbb0 (via exec wrapper 0x14292f970)
// Source: local-model fn runner; static checks only, not compiled or runtime verified.
// CONFIDENCE: HIGH
if (!bHasSeenIt)
{
	NumTimesSeeingPanningTip = 0;
	return;
}
NumTimesSeeingPanningTip++;
