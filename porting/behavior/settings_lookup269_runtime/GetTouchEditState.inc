// CONFIDENCE: high
// Native 0x144d0d580; reflected bool offset 0x473.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bTouchEditEnabled;
