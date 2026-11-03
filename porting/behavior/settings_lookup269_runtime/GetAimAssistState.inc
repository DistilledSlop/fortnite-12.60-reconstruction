// CONFIDENCE: high
// Native 0x144d00af0 calls record GetAimAssistEnabled.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->GetAimAssistEnabled();
