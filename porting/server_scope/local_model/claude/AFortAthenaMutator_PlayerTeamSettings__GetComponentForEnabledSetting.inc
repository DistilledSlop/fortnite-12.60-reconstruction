// CONFIDENCE: HIGH. Original address: 0x14281bbc0 (exec thunk). Helper: thunk_FUN_141aee110 (captured, read in full).
// Behaviour: find the scope settings object that owns PropertyName. The current scope is tried first; the default
// scope is only tried when bFallbackToDefaultTeam is set. The owner must be a FortPlayerTeamSettingsComponent.
UObject* EnabledSettingOwner = nullptr;
if (CurrentScopeSettings != nullptr && CurrentScopeSettings->SettingNameToIndex.Find(PropertyName) != INDEX_NONE)
{
    EnabledSettingOwner = CurrentScopeSettings;
}
else if (bFallbackToDefaultTeam && DefaultScopeSettings != nullptr && DefaultScopeSettings->SettingNameToIndex.Find(PropertyName) != INDEX_NONE)
{
    EnabledSettingOwner = DefaultScopeSettings;
}

if (EnabledSettingOwner == nullptr)
{
    return nullptr;
}
if (!EnabledSettingOwner->IsA(UFortPlayerTeamSettingsComponent::StaticClass()))
{
    return nullptr;
}
return EnabledSettingOwner;
