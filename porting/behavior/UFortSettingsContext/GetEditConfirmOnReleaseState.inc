// CONFIDENCE: high
// Native 0x144d03bd0; reflected bool offset 0x474.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bEditConfirmOnReleaseEnabled;
