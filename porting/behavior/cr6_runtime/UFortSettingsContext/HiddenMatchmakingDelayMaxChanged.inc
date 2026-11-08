// Original address: 0x144d12a80 (thunk 0x1452cf430)
// Offset evidence: no field access. The non-null record receives inValue via UFortClientSettingsRecord::SetHiddenMatchmakingDelayMax (0x141e3b9b0, tail-jump in the original).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    SettingsRecord->SetHiddenMatchmakingDelayMax(inValue);
}
