// Behaviour snippet: AFortPlayerStateAthena::OnRep_TotalPlayerScore (body only; spliced into the generated member function).
// Original address: 0x142a8dec0 (exec record of the RepNotify; its tail jump reaches the body at 0x141a53d60).
// Decompile source: porting/generated/decomp_improved/AFortPlayerStateAthena/OnRep_TotalPlayerScore.c (body bytes: runtime dump at 0x141a53d60)
// Proxy score: none (tenth pass; not a portability record)
// Inferred: P_FINISH is the script epilogue of the exec thunk and is not ported. The asm body is a single tail call:
//   TotalPlayerScoreNative (+0x1180) is broadcast with (this, TotalPlayerScore) through 0x1407ac640 (the decompile showed the
//   same broadcast inlined, with its lock count at +0x1194). It does not call UpdateTotalPlayerScore.
//   Name: TotalPlayerScoreNative is medium (ninth pass).
// Declared in generated header: yes
// CONFIDENCE: HIGH
TotalPlayerScoreNative.Broadcast(this, TotalPlayerScore);
