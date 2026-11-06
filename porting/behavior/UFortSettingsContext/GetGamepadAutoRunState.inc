// CONFIDENCE: high
// Native 0x144d03df0; reflected bool offset 0x47e.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bUseGamepadAutoRun;
