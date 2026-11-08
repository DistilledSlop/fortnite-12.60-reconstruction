// Original body: 0x144f22900 (thunk 0x1451f26a0). Offset evidence: MOVZX EAX,byte ptr [RCX+0xbb0]; RET. Offset 0xbb0 is the first byte after the reflected AlterationOption struct (+0xb78, size 0x38), so it is an unreflected byte in the class's own region: member field NativeRecoveredUint8_0xbb0.
// CONFIDENCE: MEDIUM
return static_cast<EFortAlterationOptionType>(NativeRecoveredUint8_0xbb0);
