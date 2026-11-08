// Original body 0x144d21bd0 (thunk 0x1452d2c60). Offset evidence: no object field access. The settings record from NativeGetSettingsRecord144d03d50 (0x144d03d50) is null-checked, then SetInvertedYaw (0x141e3bc20) is tail-called with RDX = bInChecked.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
if (Record != nullptr)
{
    Record->SetInvertedYaw(bInChecked);
}
