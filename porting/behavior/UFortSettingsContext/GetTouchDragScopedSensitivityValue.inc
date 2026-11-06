// CONFIDENCE: high
// Native 0x144d0d520; record offset 0x328.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->TouchScopedSlow : 0.0f;
