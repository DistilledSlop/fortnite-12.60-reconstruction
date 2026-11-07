// Original body address: 0x141b046e0 (thunk 0x14281bf20)
// Offset evidence: this+0x3a0 = CurrentScopeSettings (AFortAthenaMutator_PlayerSettingsBase, Object). Getter 0x1428165c0 =
// UFortPlayerTeamSettingsComponent::StaticClass(); ClassPrivate IsA check on the component. Component+0x260 = TeamIndex (Byte).
// Any failed check returns 0xff (MOV AL,0xff). Requires the UFortPlayerTeamSettingsComponent header.
// CONFIDENCE: MEDIUM
if (CurrentScopeSettings != nullptr && CurrentScopeSettings->IsA(UFortPlayerTeamSettingsComponent::StaticClass()))
{
    return static_cast<const UFortPlayerTeamSettingsComponent*>(CurrentScopeSettings)->TeamIndex;
}
return 0xff;
