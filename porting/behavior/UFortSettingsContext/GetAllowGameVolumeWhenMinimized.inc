// CONFIDENCE: high
// Native 0x144d00b10.
const UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
return Record ? Record->AllowAudioInBackground : static_cast<EFortAllowBackgroundAudioSetting>(0);
