// CONFIDENCE: high
// Native 0x144d040d0.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->GetHUDScaleRaw() * Record->GetHUDScaleMultiplier() : 1.0f;
