// Behaviour snippet: UPerkIntroScreenWidget::StartPerkScreenIntro (wrapper body; spliced into the generated member function).
// Original body address: 0x144bffd00 (thunk 0x1451ffd30, receiver delta 0, this delta total 0).
// Offset evidence: XOR EDX,EDX (no params); ADD RCX,0x258 -> OnPerkIntroFinishedEvent (reflected at +0x258; FOnPerkIntroFinished()); tail JMP to the shared broadcast.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnPerkIntroFinishedEvent.Broadcast();
