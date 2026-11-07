// Behaviour snippet: UCreativeHUDLContext::ForwardOnCreativeQuickbarActiveChanged (wrapper body; spliced into the generated member function).
// Original body address: 0x144cc60f0 (thunk 0x1451e14b0, receiver delta 0, this delta total 0).
// Offset evidence: MOV [RSP+0x38],DL stores bIsActive as a bool param; ADD RCX,0x100 -> OnCreativeQuickbarActiveChanged (reflected at +0x100; FOnCreativeQuickbarActiveChanged(bool)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnCreativeQuickbarActiveChanged.Broadcast(bIsActive);
