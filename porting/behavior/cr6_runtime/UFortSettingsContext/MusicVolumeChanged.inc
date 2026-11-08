// Original address: 0x144d14dc0 (thunk 0x1452cfa90)
// Offset evidence: no field access. The non-null record receives NewValue via UFortClientSettingsRecord::SetMusicVolume (0x141e3c210; its parameter is named InVolume in the generated header).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    SettingsRecord->SetMusicVolume(NewValue);
}
