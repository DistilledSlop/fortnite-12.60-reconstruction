// CONFIDENCE: high
// Native 0x144d03c90; reflected bool offset 0x47b.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bUseFirstPersonCamera;
