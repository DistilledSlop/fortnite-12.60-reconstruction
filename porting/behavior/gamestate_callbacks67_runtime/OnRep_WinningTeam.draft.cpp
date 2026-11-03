// Exec 0x142920880. Typed behavioral draft; offset/helper unresolved.
void AFortGameStateAthena::OnRep_WinningTeam()
{
    // Reflected setter 0x142919cc0 confirms bSkipWinnerAnnounced at +0x12d0.
    if (!bSkipWinnerAnnounced && WinningTeam > 3)
        UnresolvedProcessWinnerEvent_142919d10(this); // FindFunctionChecked(global FName), ProcessEvent(this, function, nullptr)
    WinningTeamAnnounced.Broadcast(WinningTeam);
}
