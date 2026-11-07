// Behaviour snippet: UAthenaSpectatorNameplateBase::OnPlayerPawnChanged (body only; spliced into the generated member function).
// Original address: 0x144c3b770 (thunk 0x1451c2850).
// asm: MOV [+0x2a0],R8. R8 is the 2nd declared param (InPlayerData) by ABI position, not InPlayerPawn.
// +0x2a0 is IndicatedActor (AActor*), reflected on the parent UFortActorIndicatorWidget (in this class chain).
// CONFIDENCE: MEDIUM
IndicatedActor = (AActor*)InPlayerData;
