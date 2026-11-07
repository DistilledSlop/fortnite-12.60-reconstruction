// Behaviour snippet: UAthenaRemotePlayerViewData::GetCurrentWeapon (body only; spliced into the generated member function).
// Original body: 0x144c20f90 (thunk 0x1451b8b90). Offset evidence: PlayerState at +0x1d0 (reflected). When non-null the pawn is PlayerState->GetCurrentPawn() (AFortPlayerState::GetCurrentPawn callee) and the weapon is pawn+0x588 = AFortPawn::CurrentWeapon. Otherwise CachedPlayerPawn at +0x228 (reflected) is read, a null pawn returns null, and the weapon is pawn+0x588. The asm has no null check after GetCurrentPawn, and none is added here.
// CONFIDENCE: HIGH
if (PlayerState != nullptr)
{
    return PlayerState->GetCurrentPawn()->CurrentWeapon;
}
if (CachedPlayerPawn == nullptr)
{
    return nullptr;
}
return CachedPlayerPawn->CurrentWeapon;
