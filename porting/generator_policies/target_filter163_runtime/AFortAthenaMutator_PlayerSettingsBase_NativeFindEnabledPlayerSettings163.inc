// CONFIDENCE: high
// 0x141aee020. Current scope takes precedence over the optional default scope.
UFortPlayerSettingsComponentBase* Settings = nullptr;
if (CurrentScopeSettings && CurrentScopeSettings->NativeEnabledSettingNames163.Contains(Name))
{
    Settings = CurrentScopeSettings;
}
else if (bIncludeDefault && DefaultScopeSettings && DefaultScopeSettings->NativeEnabledSettingNames163.Contains(Name))
{
    Settings = DefaultScopeSettings;
}
return Cast<UFortPlayerSettingsComponentBase>(Settings);
