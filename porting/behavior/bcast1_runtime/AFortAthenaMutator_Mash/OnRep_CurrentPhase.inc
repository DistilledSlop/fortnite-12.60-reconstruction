// Behaviour snippet: AFortAthenaMutator_Mash::OnRep_CurrentPhase (wrapper body; spliced into the generated member function).
// Original body address: 0x141ab4b00 (thunk 0x1427e9e70, receiver delta 0, this delta total 0).
// Offset evidence: MOVZX EAX,byte [RCX+0x12c0] (reflected CurrentPhase, TEnumAsByte<EMashPhase::Type>) zero-extended into a dword param at offset 0.
// Offset evidence: ADD RCX,0x290 -> OnPhaseChanged (reflected at +0x290; FOnMashPhaseChanged(int32 NewPhase)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnPhaseChanged.Broadcast(static_cast<int32>(static_cast<EMashPhase::Type>(CurrentPhase)));
