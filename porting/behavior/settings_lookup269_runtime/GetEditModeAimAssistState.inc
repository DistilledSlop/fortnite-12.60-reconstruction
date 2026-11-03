// CONFIDENCE: high
// Native 0x144d03bf0; reflected bool offset 0x47c.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bUseGamepadEditModeAimAssist;
