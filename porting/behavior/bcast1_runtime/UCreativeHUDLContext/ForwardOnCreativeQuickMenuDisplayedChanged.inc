// Behaviour snippet: UCreativeHUDLContext::ForwardOnCreativeQuickMenuDisplayedChanged (wrapper body; spliced into the generated member function).
// Original body address: 0x144cc60d0 (thunk 0x1451e1420, receiver delta 0, this delta total 0).
// Offset evidence: MOV [RSP+0x38],DL stores bIsDisplayed as a bool param; ADD RCX,0xc0 -> OnCreativeQuickMenuDisplayedChanged (reflected at +0xc0; FOnCreativeQuickMenuDisplayedChanged(bool)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnCreativeQuickMenuDisplayedChanged.Broadcast(bIsDisplayed);
