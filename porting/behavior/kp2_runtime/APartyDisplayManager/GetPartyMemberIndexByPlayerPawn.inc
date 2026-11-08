// Behaviour snippet: APartyDisplayManager::GetPartyMemberIndexByPlayerPawn (body only; spliced into the generated member function).
// Original body address: 0x141fc1d10 (thunk 0x142b7e340)
// Offset evidence: +0x220 = PlacementActors.Num() (loop bound); +0x298 HeroPlayerPawns TMap<int32,FHeroSlotInfo> is probed per index (0x180-byte value stride).
// The probe result register RBP is zeroed at entry (XOR EBP,EBP) and never reassigned in the body, so the pawn comparison is never reached.
// Every iteration falls through, the output index stays -1 and the function returns false. The loop reads have no side effects.
// CONFIDENCE: MEDIUM
(void)PlayerPawn;
OutPlayerIndex = -1;
return false;
