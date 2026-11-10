// CONFIDENCE: LOW. Impl at 0x1423abe70 is fully read. FUN_142761080 and FUN_142a92a00 are not read.
if (DefenderTrap != nullptr)
{
    APlayerState* PlayerState = this->PlayerState;   // this+0x220
    if (PlayerState != nullptr && PlayerState->IsA(PlayerStateSubclass)   // class from FUN_142a92a00, unnamed)
    {
        RemoveDefenderFromTrap(DefenderTrap, PlayerState, false);   // FUN_142761080(trap, player, 0)
    }
}
