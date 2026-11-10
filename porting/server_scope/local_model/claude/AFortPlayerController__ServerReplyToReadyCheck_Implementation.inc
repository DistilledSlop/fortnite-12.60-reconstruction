// CONFIDENCE: MEDIUM. Impl at 0x1423abf10 is fully read. FUN_14242a0a0 is not read.
APlayerState* PlayerState = this->PlayerState;   // this+0x220
if (PlayerState != nullptr && PlayerState->IsA(AFortPlayerState::StaticClass()))
{
    SetReadyCheckState(PlayerState, bReady ? 1 : 2);   // FUN_14242a0a0(player, (bReady ^ 1) + 1)
}
