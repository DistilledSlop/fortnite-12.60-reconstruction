// Original address: 0x144d007f0 (thunk 0x1452cd7a0)
// Offset evidence: no field access. NativeGetSettingsRecord144d03d50 returns the settings record (RCX stays this, receiver delta 0); the non-null record receives NewValue via UFortClientSettingsRecord::SetGamepadLookStickDeadZone (0x141e3b680).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    SettingsRecord->SetGamepadLookStickDeadZone(NewValue);
}
