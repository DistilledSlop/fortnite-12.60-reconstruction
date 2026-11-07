// Behaviour snippet: AFortMinigame::HandleMinigameStarted_Implementation (wrapper body; spliced into the generated member function).
// Original body address: 0x142098600 (thunk 0x1429c27e0, receiver delta 0, this delta total 0).
// Offset evidence: XOR EDX,EDX (no params); ADD RCX,0x378 -> MinigameStartedDelegate (reflected at +0x378; FSimpleMinigameDelegate()); tail JMP.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
MinigameStartedDelegate.Broadcast();
