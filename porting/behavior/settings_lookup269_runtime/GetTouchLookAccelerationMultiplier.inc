// CONFIDENCE: high
// Native 0x144d0d5a0; record offset 0x31c.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->TouchLookAccelerationMultiplier : 1.0f;
