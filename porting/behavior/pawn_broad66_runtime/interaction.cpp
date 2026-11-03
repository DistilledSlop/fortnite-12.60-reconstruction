// Rough lift of execs 0x142a80c90, 0x142a7b6a0, 0x142a7e1c0.
// The emitted C inlines FMulticastScriptDelegate::Broadcast: it snapshots the
// 16-byte invocation list, executes live bound functions, and removes stale
// entries from the original list with swap removal. Broadcast supplies that
// behavior, including mutation/reentrancy handling.
void AFortPlayerPawn::StartedInteractSearch()
{
    bStartedInteractSearch = true;
    OnStartedInteractSearch.Broadcast();
}
void AFortPlayerPawn::EndedInteractSearch()
{
    bStartedInteractSearch = false;
    OnEndedInteractSearch.Broadcast();
}
void AFortPlayerPawn::OnRep_StartedInteractSearch()
{
    if (bStartedInteractSearch) OnStartedInteractSearch.Broadcast();
    else OnEndedInteractSearch.Broadcast();
}
