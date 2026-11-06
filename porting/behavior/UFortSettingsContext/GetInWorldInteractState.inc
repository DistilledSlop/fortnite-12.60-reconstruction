// CONFIDENCE: high
// Native 0x144d04c60.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && static_cast<uint8>(Record->TouchInteractionMode) == 1;
