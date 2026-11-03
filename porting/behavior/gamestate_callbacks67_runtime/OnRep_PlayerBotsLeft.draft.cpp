// Exec 0x1429205d0. Behavioral draft; helper names and hidden fields unresolved.
void AFortGameStateAthena::OnRep_PlayerBotsLeft()
{
    UWorld* World = GetWorld();
    // Option byte at runtime options +0x778; original only obtains World if enabled.
    if (UnresolvedOptionAt778() && World)
    {
        auto& Timers = UnresolvedWorldTimerManager(World); // world+0x188 ? +0xc0 : world+0x320
        auto& Handle = UnresolvedTimerHandleAt1420(*this); // original this+0x1420
        if (UnresolvedTimerMissingOrState2(Timers, Handle))
            UnresolvedSetTimer_1446278d0(Timers, Handle, this, 0x14199cef0);
    }
    const int32 Total = PlayerBotsLeft + PlayersLeft;
    // Original +0x1efc; LobbyAction is at a different offset in source layout.
    if (uint8(GamePhase) == 4 && Total <= UnresolvedIntAt1efc(*this))
    {
        UnresolvedPhaseThresholdEffect_141e9a1a0(*this);
        auto Value = UnresolvedVirtualA70(*this);
        UnresolvedPhaseValueEffect_141eb30f0(*this, Value);
        UnresolvedPublishAthenaGamePhase(Value);
    }
    OnPlayersLeftChanged.Broadcast(Total);
}
