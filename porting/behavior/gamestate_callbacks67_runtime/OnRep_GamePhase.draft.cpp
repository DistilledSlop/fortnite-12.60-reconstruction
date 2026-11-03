// Exec 0x142920470 decodes OldGamePhase (uint8) and calls this native body.
// No behavioral expansion possible from exec decomp alone.
void AFortGameStateAthena::OnRep_GamePhase(EAthenaGamePhase OldGamePhase)
{
    // Wrapper 0x1419a65b0 passes both arguments unchanged to 0x14199ac00;
    // surrounding STAT_InvNPerfHook_GSA_RGP instrumentation omitted.
    UnresolvedGamePhaseBody_14199ac00(*this, OldGamePhase);
}
