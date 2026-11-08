// Original address: 0x144d12ab0 (thunk 0x1452cf4b0)
// Offset evidence: no field access. The non-null record receives 0 (XOR EDX,EDX) via UFortClientSettingsRecord::SetHiddenMatchmakingDelayMax (0x141e3b9b0, tail-jump in the original).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    SettingsRecord->SetHiddenMatchmakingDelayMax(0);
}
