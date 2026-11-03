// CONFIDENCE: high
// Native 0x144d09bc0; record offset 0x2e8.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->ScopedSlow : 0.0f;
