// CONFIDENCE: high
// Native 0x144d04100; record offset 0x278.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->HUDScale : 1.0f;
