// Behaviour snippet: UFortDialogNotificationHandler::Accept_Implementation (wrapper body; spliced into the generated member function).
// Original body address: 0x141d3a2d0 (thunk 0x1428dba70, receiver delta 0, this delta total 0).
// Offset evidence: XOR EDX,EDX (no params); ADD RCX,0x40 -> OnAccepted (reflected at +0x40; FOnActionComplete()); tail JMP to the shared broadcast. Job fn name is Accept; entry is Accept_Implementation.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnAccepted.Broadcast();
