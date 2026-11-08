// Original body 0x144d20e50 (thunk 0x1452d2740). Offset evidence: no object field access. The settings record from NativeGetSettingsRecord144d03d50 (0x144d03d50) is null-checked, then SetTargetingToggleable (0x141e3cac0) is tail-called with RDX = bInChecked.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
if (Record != nullptr)
{
    Record->SetTargetingToggleable(bInChecked);
}
