// Behaviour snippet: UCommonWidgetSwitcher::HandleActiveWidgetDeactivated (wrapper body; spliced into the generated member function).
// Original body address: 0x1407c6660 (thunk 0x1408184c0, receiver delta 0, this delta total 0).
// Offset evidence: ADD RCX,0x190 -> OnActiveWidgetDeactivated (reflected at +0x190; FOnActiveWidgetDeactivated(UCommonActivatablePanel* DeactivatedWidget)).
// Offset evidence: RDX (DeactivatedPanel) stored at param offset 0; TEST RDX,RDX / JZ skips the store and the call when null.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
if (DeactivatedPanel != nullptr)
{
    OnActiveWidgetDeactivated.Broadcast(DeactivatedPanel);
}
