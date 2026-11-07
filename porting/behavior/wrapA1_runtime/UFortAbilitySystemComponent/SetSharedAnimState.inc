// Behaviour snippet: UFortAbilitySystemComponent::SetSharedAnimState (wrapper body 0x14160d3b0, thunk 0x1427819a0).
// Offset evidence: MOV qword [this+0x14e0],0 is RepSharedAnimInfo.AnimMontage (field at +8 of the FGameplayAbilityRepSharedAnim struct at this+0x14d8); MOV byte [this+0x14d8],DL is RepSharedAnimInfo.AnimState (field at +0, arg State in DL); MOV byte [this+0x14d9],0 is RepSharedAnimInfo.MontageSectionToPlay (field at +1).
// Tail JMP 0x141609170 is UFortAbilitySystemComponent::OnRep_SharedAnim() (declared in generated UFortAbilitySystemComponent.h, no params, RCX unchanged = this).
// CONFIDENCE: HIGH
RepSharedAnimInfo.AnimMontage = nullptr;
RepSharedAnimInfo.AnimState = State;
RepSharedAnimInfo.MontageSectionToPlay = 0;
OnRep_SharedAnim();
