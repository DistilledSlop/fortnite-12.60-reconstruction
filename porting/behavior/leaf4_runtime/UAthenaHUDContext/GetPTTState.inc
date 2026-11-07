// Behaviour snippet: UAthenaHUDContext::GetPTTState (body only; spliced into the generated member function).
// Original address: 0x144c8f670 (thunk 0x14519dce0).
// asm: MOVZX EAX,byte [+0x260]; RET (enum byte). PlayerViewModel (Object) ends at +0x260; NativeRecoveredInt_0x264 exists.
// CONFIDENCE: MEDIUM
return static_cast<EPTTState>(NativeRecoveredByte_0x260);
