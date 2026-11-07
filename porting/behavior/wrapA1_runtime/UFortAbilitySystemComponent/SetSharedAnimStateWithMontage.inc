// Behaviour snippet: UFortAbilitySystemComponent::SetSharedAnimStateWithMontage (wrapper body 0x14160d3d0, thunk 0x142781a20).
// Offset evidence: MOV qword [this+0x14e0],R8 is RepSharedAnimInfo.AnimMontage (field at +8 of the struct at this+0x14d8; arg Montage in R8); MOV byte [this+0x14d8],DL is RepSharedAnimInfo.AnimState (field at +0; arg State in DL); MOV byte [this+0x14d9],0 is RepSharedAnimInfo.MontageSectionToPlay (field at +1).
// Tail JMP 0x141609170 is UFortAbilitySystemComponent::OnRep_SharedAnim() (declared in generated UFortAbilitySystemComponent.h, no params, RCX unchanged = this).
// CONFIDENCE: HIGH
RepSharedAnimInfo.AnimMontage = Montage;
RepSharedAnimInfo.AnimState = State;
RepSharedAnimInfo.MontageSectionToPlay = 0;
OnRep_SharedAnim();
