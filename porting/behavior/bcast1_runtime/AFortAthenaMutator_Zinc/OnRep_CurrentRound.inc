// Behaviour snippet: AFortAthenaMutator_Zinc::OnRep_CurrentRound (wrapper body; spliced into the generated member function).
// Original body address: 0x141b4ee60 (thunk 0x14282e8d0, receiver delta 0, this delta total 0).
// Offset evidence: MOV [RSP+0x20],RCX stores the receiver (this) at offset 0; MOV EAX,dword [RCX+0x608] (reflected CurrentRound int32) stored at offset 8.
// Offset evidence: ADD RCX,0x518 -> OnRoundAdvanced (reflected at +0x518; FZincRoundAdvanced(AFortAthenaMutator_Zinc* Mutator, int32 Round)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnRoundAdvanced.Broadcast(this, CurrentRound);
