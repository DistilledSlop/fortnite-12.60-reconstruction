// Behaviour snippet: UFortHUDContext::HandlePersonalVehicleModeChanged (wrapper body; spliced into the generated member function).
// Original body address: 0x144cd4810 (thunk 0x1452576e0, receiver delta 0, this delta total 0).
// Offset evidence: MOV [RSP+0x38],DL stores bEnteredVehicle as a bool param; ADD RCX,0x4e0 -> OnPersonalVehicleModeChanged (reflected at +0x4e0; FOnPersonalVehicleModeChange(bool)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnPersonalVehicleModeChanged.Broadcast(bEnteredVehicle);
