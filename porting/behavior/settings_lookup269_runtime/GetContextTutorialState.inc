// CONFIDENCE: high
// Native 0x144d02a50; reflected bool offset 0x478.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bContextTutorialEnabled;
