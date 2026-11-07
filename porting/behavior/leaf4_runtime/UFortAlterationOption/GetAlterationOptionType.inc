// Behaviour snippet: UFortAlterationOption::GetAlterationOptionType (body only; spliced into the generated member function).
// Original address: 0x144f22900 (thunk 0x1451f26a0).
// asm: MOVZX EAX,byte [+0xbb0]; RET (enum byte). FAlterationOption at +0xb78 is 0x38 bytes (Costs at struct +0x28, 16 bytes), so +0xbb0 is just past it.
// CONFIDENCE: MEDIUM
return static_cast<decltype(this->GetAlterationOptionType(0))>(NativeRecoveredByte_0xbb0);
