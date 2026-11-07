// Behaviour snippet: UFortHUDContext::HandleToggledBattleMapLive (body only; spliced into the generated member function).
// Original body: 0x144cd5110 (thunk 0x145257790); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV qword [RSP+0x30],RDX; ADD RCX,0x258 -> OnToggledBattleMapLive (+0x258), FOnToggledBattleMapLive(ABattleMapPawnLive*).
// CONFIDENCE: HIGH
OnToggledBattleMapLive.Broadcast(NewSpectatorPawn);
