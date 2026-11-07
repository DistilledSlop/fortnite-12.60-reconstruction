// Behaviour snippet: AFortAthenaMutator_Arsenic::IsPawnAnInfected (body only; spliced into the generated member function).
// Original body address: 0x141aad120 (thunk 0x1427e1490)
// Offset evidence: MOVZX EAX,byte [this+0x441]; RET. Pawn (RDX) is not read. Returns raw InfectedTeam (uint8) as bool.
// CONFIDENCE: MEDIUM
return InfectedTeam != 0;
