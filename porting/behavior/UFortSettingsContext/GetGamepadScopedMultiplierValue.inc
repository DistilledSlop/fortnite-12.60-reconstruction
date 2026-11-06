// CONFIDENCE: high
// Native 0x144d03f70; record offset 0x2f0.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->GamepadScopedSlow : 0.0f;
