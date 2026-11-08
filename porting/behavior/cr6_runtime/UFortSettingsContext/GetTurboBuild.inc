// Original address: 0x144d0dbe0 (thunk 0x1452cf260)
// Offset evidence: no field access. Null record returns false (XOR AL,AL); otherwise the result of UFortClientSettingsRecord::GetTurboBuild (0x141e2c2c0) is returned as a bool.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (!SettingsRecord)
{
    return false;
}
return SettingsRecord->GetTurboBuild();
