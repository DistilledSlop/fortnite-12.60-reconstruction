// CONFIDENCE: high
// Native 0x144d07a40; reflected bool offset 0x475.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bQuickEditEnabled;
