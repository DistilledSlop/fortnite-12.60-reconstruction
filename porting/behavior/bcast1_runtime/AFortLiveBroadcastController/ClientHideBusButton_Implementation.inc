// Behaviour snippet: AFortLiveBroadcastController::ClientHideBusButton_Implementation (wrapper body; spliced into the generated member function).
// Original body address: 0x142371dd0 (thunk 0x1429838f0, receiver delta 0, this delta total 0).
// Offset evidence: XOR EDX,EDX (no params); ADD RCX,0x2da8 -> OnHideBusButtonFromServer (reflected at +0x2da8; FNoParamDelegate()); tail JMP.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnHideBusButtonFromServer.Broadcast();
