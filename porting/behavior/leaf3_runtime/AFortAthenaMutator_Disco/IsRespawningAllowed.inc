// Behaviour snippet: AFortAthenaMutator_Disco::IsRespawningAllowed (body only; spliced into the generated member function).
// Original body address: 0x141adb950 (thunk 0x1427fe350)
// Offset evidence: MOV RAX,[this+0x260] (CachedGameState); MOVZX EDX,byte [RAX+0x1eb9] (SafeZonePhase); CMP EDX,dword [this+0x3f0] (LastRespawnableSafeZoneIndex); SETLE.
// CONFIDENCE: HIGH
AFortGameStateAthena* GS = CachedGameState;
return static_cast<int32>(GS->SafeZonePhase) <= LastRespawnableSafeZoneIndex;
