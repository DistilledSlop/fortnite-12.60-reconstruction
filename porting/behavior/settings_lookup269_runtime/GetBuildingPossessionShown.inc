// CONFIDENCE: high
// Native 0x144d015f0; reflected bool offset 0x479.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bBuildingPossessionShown;
