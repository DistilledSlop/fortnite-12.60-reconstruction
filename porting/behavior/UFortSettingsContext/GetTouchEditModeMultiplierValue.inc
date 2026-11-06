// CONFIDENCE: high
// Native 0x144d0d550; record offset 0x330.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->TouchEditModeMultiplier : 0.0f;
