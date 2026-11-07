// Behaviour snippet: UAthenaHUDContext::SetPostGameWidgetPlacement (wrapper body; spliced into the generated member function).
// Original body address: 0x144cace60 (thunk 0x14519fbe0, receiver delta 0, this delta total 0).
// Offset evidence: RDX (WidgetPlacement, FName) stored at param offset 0.
// Offset evidence: ADD RCX,0x1d0 -> OnPostGameWidgetPlacementChange (reflected at +0x1d0; FOnPostGameWidgetPlacement(FName)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnPostGameWidgetPlacementChange.Broadcast(WidgetPlacement);
