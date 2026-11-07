// Behaviour snippet: AFortPlayerState::GetCurrentPawn (body only).
// Original address: 0x1419b6d20 (thunk 0x142a88a40)
// Offset evidence: this+0x278 = APlayerState::PawnPrivate (reflected Object, APawn*); returned only when it IsA AFortPlayerPawn (static getter 0x142a75f10), else null.
// CONFIDENCE: HIGH
APawn* Pawn = PawnPrivate;
if (Pawn != nullptr && Pawn->IsA(AFortPlayerPawn::StaticClass()))
{
    return static_cast<AFortPlayerPawn*>(Pawn);
}
return nullptr;
