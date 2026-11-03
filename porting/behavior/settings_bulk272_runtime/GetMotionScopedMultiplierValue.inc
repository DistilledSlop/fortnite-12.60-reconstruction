// CONFIDENCE: high
// Native 0x144d06570.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->MotionScopedSlow : 0.0f;
