// CONFIDENCE: LOW. Impl at 0x1423ac090 is read in full, but FUN_140f95ae0, FUN_144621fb0, FUN_1446278d0 and the slot 0x1510/0xa58 targets are not read.
// Base-class slot 0x1328 resolves to 0x1423ac090; the Athena override 0x1406801c0 is not used here.
if (!IsActiveSessionMember(this->SessionId))   // FUN_144621fb0 with the session context from FUN_140f95ae0; active means found and state != 2
{
    RegisterReturnToMenuRequest(this->SessionId);   // FUN_1446278d0; the request object is a heap-allocated delegate
}
AActor* World = GetWorld();   // virtual slot 0x150
if (APlayerState* PlayerState = this->PlayerState)   // this+0x220
{
    // Notify the authority game mode that this player has returned (virtual slot 0xaa0; not read).
}
// Log at verbosity 5 or above: label is [client], or [sessionowner] when the player state is in the owning session.
FinishReturnToMainMenu(this, GetLogContext());   // virtual slot 0xa58 on this; FUN_1407c2c80 returns the log context
