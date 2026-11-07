// Behaviour snippet: UFortQuestTrackerList::HandleSizeEstimateChanged (wrapper body; spliced into the generated member function).
// Original body address: 0x144fd75e0 (thunk 0x1452b9fd0, receiver delta 0, this delta total 0).
// Offset evidence: CMP dword [RCX+0x288],0; JLE skips the store and the call. +0x288 is +8 inside reflected OnSizeEstimateChangedDelegate (+0x280, 16-byte multicast storage), so it is read raw as the invocation-list count.
// Offset evidence: MOV [RSP+0x30],RCX stores the receiver (this) at param offset 0; ADD RCX,0x280 -> OnSizeEstimateChangedDelegate (FOnSizeEstimateChanged(UObject* ChangedElement)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
if (*reinterpret_cast<const int32*>(Raw + 0x288) > 0)
{
    OnSizeEstimateChangedDelegate.Broadcast(this);
}
