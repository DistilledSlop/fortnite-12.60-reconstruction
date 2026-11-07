// Behaviour snippet: UCreativeHUDLContext::HandleOnCreativeModeEnabledChanged (wrapper body; spliced into the generated member function).
// Original body address: 0x144cd43a0 (thunk 0x1451e2030, receiver delta 0, this delta total 0).
// Offset evidence: MOV [RSP+0x38],DL stores bIsEnabled as a bool param; ADD RCX,0xf0 -> OnCreativeModeEnabledChanged (reflected at +0xf0; FOnCreativeModeEnabledChanged(bool)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnCreativeModeEnabledChanged.Broadcast(bIsEnabled);
