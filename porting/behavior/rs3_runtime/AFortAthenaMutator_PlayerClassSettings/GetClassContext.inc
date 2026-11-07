// Original body address: 0x141b01b00 (thunk 0x142814ed0)
// Offset evidence: this+0x3a0 = CurrentScopeSettings (AFortAthenaMutator_PlayerSettingsBase, Object). Getter 0x1428123a0 =
// UFortPlayerClassSettingsComponent::StaticClass(); ClassPrivate IsA check on the component. Component+0x260 = ClassSlotIndex (Byte).
// Any failed check returns 0xff (MOV AL,0xff). Requires the UFortPlayerClassSettingsComponent header.
// CONFIDENCE: MEDIUM
if (CurrentScopeSettings != nullptr && CurrentScopeSettings->IsA(UFortPlayerClassSettingsComponent::StaticClass()))
{
    return static_cast<const UFortPlayerClassSettingsComponent*>(CurrentScopeSettings)->ClassSlotIndex;
}
return 0xff;
