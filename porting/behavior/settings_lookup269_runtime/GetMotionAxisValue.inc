// CONFIDENCE: high
// Native 0x144d06480.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? static_cast<int32>(Record->MotionYawAxis) : 0;
