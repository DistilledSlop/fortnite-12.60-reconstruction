// Behaviour snippet: AFortPlayerPawn::OnRep_DBNOHoistee (body only).
// Original address: 0x142339cb0 (thunk 0x142a7dc70)
// Offset evidence: this+0x250 = APawn::Controller (reflected Object); this+0x1f58 = AFortPlayerPawn::DBNOHoistee (reflected Object);
// controller+0x2470 = AFortPlayerControllerGameplay::OnDBNOCarryPlayerChanged (reflected InlineMulticastDelegate, one bool param).
// Class gate: AFortPlayerControllerGameplay::StaticClass() (static getter 0x142a62c80) via IsA.
// CONFIDENCE: HIGH
if (Controller != nullptr && Controller->IsA(AFortPlayerControllerGameplay::StaticClass()))
{
    AFortPlayerControllerGameplay* GameplayController = static_cast<AFortPlayerControllerGameplay*>(Controller);
    GameplayController->OnDBNOCarryPlayerChanged.Broadcast(DBNOHoistee != nullptr);
}
