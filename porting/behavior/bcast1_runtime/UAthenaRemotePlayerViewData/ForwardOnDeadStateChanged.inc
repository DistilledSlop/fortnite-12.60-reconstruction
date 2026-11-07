// Behaviour snippet: UAthenaRemotePlayerViewData::ForwardOnDeadStateChanged (wrapper body; spliced into the generated member function).
// Original body address: 0x144c1fe80 (thunk 0x1451b8300, receiver delta 0, this delta total 0).
// Offset evidence: MOV [RSP+0x20],RCX stores the receiver (this) at param offset 0 before the ADD; RDX (InPlayerState) is never read; R8B (bInIsDead) stored at offset 8.
// Offset evidence: ADD RCX,0xd8 -> OnDeadStateChanged (reflected at +0xd8; FBoolDelegate(UAthenaRemotePlayerViewData* PlayerData, bool BoolParam)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: MEDIUM
OnDeadStateChanged.Broadcast(this, bInIsDead);
