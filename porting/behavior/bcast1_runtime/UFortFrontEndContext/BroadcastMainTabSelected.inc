// Behaviour snippet: UFortFrontEndContext::BroadcastMainTabSelected (wrapper body; spliced into the generated member function).
// Original body address: 0x144cc1480 (thunk 0x145238f70, receiver delta 0, this delta total 0).
// Offset evidence: RDX (TabName, FName) stored at param offset 0; ADD RCX,0x1a8 -> OnMainTabSelected (reflected at +0x1a8; FOnMainTabSelectedDelegate(FName TabName)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnMainTabSelected.Broadcast(TabName);
