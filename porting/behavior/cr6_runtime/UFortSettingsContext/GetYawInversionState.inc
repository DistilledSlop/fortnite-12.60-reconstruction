// Original address: 0x144d0f780 (thunk 0x1452cf380)
// Offset evidence: no field access. Null record returns false; otherwise the result of UFortClientSettingsRecord::GetInvertedYaw (0x141e27600) is returned, normalised to a bool (TEST AL,AL / MOV AL,1).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (!SettingsRecord)
{
    return false;
}
return SettingsRecord->GetInvertedYaw();
