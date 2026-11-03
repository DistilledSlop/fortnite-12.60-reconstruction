// CONFIDENCE: high
// Native 0x144d06620.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->MotionTargetingSlow : 0.0f;
