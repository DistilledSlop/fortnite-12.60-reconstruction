// Behaviour snippet: UFortGameStateComponent_EventLevel::NotifyRestedXpGenAmount (body only; spliced into the generated member function).
// Original address: 0x141a854b0
// Evidence: ADD dword ptr [RCX+0x2a8],EDX. Offset 0x2a8 is past reflected LastSelectedNavObj (FGameplayTag at 0x298, 8 bytes); no reflected member covers it; own-region gap -> NativeRecoveredInt_0x2a8.
// CONFIDENCE: MEDIUM
NativeRecoveredInt_0x2a8 += RestedXp;
