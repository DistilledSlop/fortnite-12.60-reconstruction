// CONFIDENCE: high
// Native 0x144d04050.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? (Record->InitialGammaValue - MinGammaValue) / (MaxGammaValue - MinGammaValue) : 0.5f;
