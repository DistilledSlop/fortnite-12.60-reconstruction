// Behaviour snippet: UFortItemQuantityListBase::IsEmptyList (body only; spliced into the generated member function).
// Original address: 0x144f2cef0 (thunk 0x145278560).
// asm: MOV EAX,dword [+0x29c]; CMP dword [+0x270],EAX; SETZ AL; RET. Neither offset is reflected (gap between ListEntryWidgetType +0x260 and ItemObjectsToKeepAlive +0x2b8).
// CONFIDENCE: HIGH
return NativeRecoveredInt_0x29c == NativeRecoveredInt_0x270;
