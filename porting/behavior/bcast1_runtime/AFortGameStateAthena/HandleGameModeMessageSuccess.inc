// Behaviour snippet: AFortGameStateAthena::HandleGameModeMessageSuccess (wrapper body; spliced into the generated member function).
// Original body address: 0x14199abb0 (thunk 0x14291fb80, receiver delta 0, this delta total 0).
// Offset evidence: RDX (OwnerController) stored at param offset 0; ADD RCX,0x10e8 -> GameModeMessageSuccess (reflected at +0x10e8; FOnGameModeMessageSuccess(AFortPlayerController*)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
GameModeMessageSuccess.Broadcast(OwnerController);
