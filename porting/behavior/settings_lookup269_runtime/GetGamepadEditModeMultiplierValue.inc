// CONFIDENCE: high
// Native 0x144d03e50; record offset 0x2f8.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->GamepadEditModeMultiplier : 0.0f;
