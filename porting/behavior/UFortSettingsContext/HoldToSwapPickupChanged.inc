// Behaviour snippet: UFortSettingsContext::HoldToSwapPickupChanged (body only; spliced into the generated member function).
// Original address: 0x144d12b30 (via exec wrapper 0x1452cf560)
// Source: local-model fn runner; static checks only, not compiled or runtime verified.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord != nullptr && bInChecked != SettingsRecord->bUseHoldToSwapPickup)
{
    SettingsRecord->bUseHoldToSwapPickup = bInChecked;
    SettingsRecord->bDirty = true;
}
