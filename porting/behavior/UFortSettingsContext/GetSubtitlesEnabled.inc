// CONFIDENCE: high
// Native 0x144d0b080; reflected bool offset 0x550.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bEnableSubtitles;
