// Original address: 0x144d14df0 (thunk 0x1452cfb10)
// Offset evidence: no field access. The non-null record receives bNotify (zero-extended DL) via UFortClientSettingsRecord::SetNotifyUsersWhenPlaying (0x141e3c260, tail-jump in the original).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    SettingsRecord->SetNotifyUsersWhenPlaying(bNotify);
}
