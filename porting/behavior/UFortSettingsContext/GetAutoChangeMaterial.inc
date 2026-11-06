// CONFIDENCE: high
// Native 0x144d01170 calls record GetAutoChangeMaterial.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record && Record->GetAutoChangeMaterial();
