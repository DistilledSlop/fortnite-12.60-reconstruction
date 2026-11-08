// Original address: 0x144d0f3b0 (thunk 0x1452cf2f0)
// Offset evidence: no field access. Null record returns false (RAX=0); otherwise the body tail-jumps to UFortClientSettingsRecord::GetVoiceChatEnabled (0x141e2c560).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (!SettingsRecord)
{
    return false;
}
return SettingsRecord->GetVoiceChatEnabled();
