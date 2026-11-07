// Behaviour snippet: UFortHUDContext::RemovePointOfInterest (wrapper body; spliced into the generated member function).
// Original body address: 0x144ce5680 (thunk 0x145258080, receiver delta 0, this delta total 0).
// Offset evidence: RDX (PointOfInterest, AActor*) stored at param offset 0; ADD RCX,0x478 -> OnPointOfInterestRemoved (reflected at +0x478; FOnPointOfInterestRemoved(AActor*)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnPointOfInterestRemoved.Broadcast(PointOfInterest);
