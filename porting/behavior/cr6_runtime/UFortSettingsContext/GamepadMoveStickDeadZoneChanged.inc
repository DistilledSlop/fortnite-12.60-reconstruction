// Original address: 0x144d00820 (thunk 0x1452cd820)
// Offset evidence: no field access. NativeGetSettingsRecord144d03d50 returns the settings record (RCX stays this, receiver delta 0); the non-null record receives NewValue via UFortClientSettingsRecord::SetGamepadMoveStickDeadZone (0x141e3b6b0).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    SettingsRecord->SetGamepadMoveStickDeadZone(NewValue);
}
