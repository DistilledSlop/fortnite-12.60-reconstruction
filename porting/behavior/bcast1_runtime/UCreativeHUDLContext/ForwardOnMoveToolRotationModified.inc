// Behaviour snippet: UCreativeHUDLContext::ForwardOnMoveToolRotationModified (wrapper body; spliced into the generated member function).
// Original body address: 0x144cc66b0 (thunk 0x1451e1770, receiver delta 0, this delta total 0).
// Offset evidence: MOVUPS XMM0,[RDX] copies the 16-byte FQuat argument (passed by reference) into param offset 0.
// Offset evidence: ADD RCX,0xd0 -> OnMoveToolRotationModified (reflected at +0xd0; FOnMoveToolRotationModified(FQuat currentRotationOffset)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnMoveToolRotationModified.Broadcast(currentRotationOffset);
