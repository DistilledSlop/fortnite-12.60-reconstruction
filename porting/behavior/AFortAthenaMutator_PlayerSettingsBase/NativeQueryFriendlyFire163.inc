// CONFIDENCE: high
// 0x141aff8f0. A present override returns query-result byte 2.
const FName SettingName(TEXT("bAllowFriendlyFire"));
UFortPlayerSettingsComponentBase* Settings = NativeFindEnabledPlayerSettings163(SettingName, true);
if (!Settings)
{
    return 0;
}
Value = Settings->bAllowFriendlyFire;
return 2;
