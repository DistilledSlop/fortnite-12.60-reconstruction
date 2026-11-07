// Behaviour snippet: UFortNotificationHandler::ClearNotification_Implementation (wrapper body; spliced into the generated member function).
// Original body address: 0x14243dd10 (thunk 0x142710860, receiver delta 0, this delta total 0).
// Offset evidence: CMP dword [RCX+0x30],0; JLE jumps to the RET and skips the broadcast. +0x30 is +8 inside reflected OnNotificationCleared (+0x28, FFortNotificationClearedDelegate()); read raw as its invocation-list count.
// Offset evidence: ADD RCX,0x28 -> OnNotificationCleared; XOR EDX,EDX; tail JMP to the shared broadcast.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
if (*reinterpret_cast<const int32*>(Raw + 0x30) > 0)
{
    OnNotificationCleared.Broadcast();
}
