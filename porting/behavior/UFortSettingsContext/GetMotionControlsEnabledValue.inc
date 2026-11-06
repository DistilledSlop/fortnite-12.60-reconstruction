// CONFIDENCE: high
// Native 0x144d064a0; reflected bool offset 0x338.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->bMotionControlsEnabled;
