// CONFIDENCE: high
// Native 0x144d07090; reflected bool offset 0x63a.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bUsePowerSavingMode;
