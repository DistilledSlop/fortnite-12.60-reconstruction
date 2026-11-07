// Behaviour snippet: UFortUISpecialEventSubsystem::SetCurrentUISpecialEventState (wrapper body; spliced into the generated member function).
// Original body address: 0x144e199d0 (thunk 0x14531a230, receiver delta 0, this delta total 0).
// Offset evidence: CMP EDX,dword [RCX+0x50]; JZ skips the store and the call when equal. MOV dword [RCX+0x50],EDX stores the 32-bit value into an unreflected gap at +0x50 (after OnTemporarySpecialUIEvent at +0x40), emitted as member_field NativeRecoveredInt32_0x50.
// Offset evidence: MOV byte [RSP+0x38],DL; ADD RCX,0x30 -> OnNewUISpecialEventState (reflected at +0x30; FSpecialUIEventDelegate(TEnumAsByte<EFortUISpecialEvents>)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: MEDIUM
const int32 NewEventValue = static_cast<int32>(InNewEventState);
if (NewEventValue != NativeRecoveredInt32_0x50)
{
    NativeRecoveredInt32_0x50 = NewEventValue;
    OnNewUISpecialEventState.Broadcast(InNewEventState);
}
