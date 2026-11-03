// CONFIDENCE: high
// Native 0x144d0ade0; reflected bool offset 0x46b.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bSprintCancelsReload;
