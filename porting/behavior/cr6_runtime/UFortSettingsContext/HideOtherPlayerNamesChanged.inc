// Original address: 0x144d12ae0 (thunk 0x1452cf4d0)
// Offset evidence: no field access. The non-null record receives bInChecked (zero-extended DL) via UFortClientSettingsRecord::SetHideOtherPlayerNames (0x141e3b9d0, tail-jump in the original).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    SettingsRecord->SetHideOtherPlayerNames(bInChecked);
}
