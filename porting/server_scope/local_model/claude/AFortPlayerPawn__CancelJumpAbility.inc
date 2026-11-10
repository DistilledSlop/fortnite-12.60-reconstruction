// CONFIDENCE: MEDIUM
// Original address: 0x142a7b160 (exec thunk, no parameters). Capture: captures/13799d83931e48b19e2b5ccf2a1528b5/0x142a7b160.c
// FUN_140708a10 = FindAbilitySpecFromHandle (read in full): scans ActivatableAbilities (+0x500, count +0x508, 200-byte specs) for Handle at spec+0xc.
if (Role != ROLE_SimulatedProxy)
{
	// Handle is an int at offset 5104 (JumpAbilityInput +8). The member name is not recovered; described here as JumpAbilityInput.Handle.
	if (JumpAbilityInput.Handle != -1)
	{
		// AbilitySystemComponent (+0x4c8) vslot 0x10 is GetAbilitySystemComponent, unnamed.
		UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
		if (ASC != nullptr)
		{
			FGameplayAbilitySpec* Spec = FindAbilitySpecFromHandle(ASC, JumpAbilityInput.Handle);
			if (Spec != nullptr && Spec->Ability != nullptr)
			{
				FGameplayAbilityActivationInfo ActivationInfo;   // zero-initialised in the decompile
				// Vslot 0x2c8 with (Handle, ActorInfo at ASC+1000, ActivationInfo, bReplicate = false). Named EndAbility, name proposed.
				Spec->Ability->EndAbility(JumpAbilityInput.Handle, ASC->ActorInfo, ActivationInfo, false);
			}
		}
	}
}
