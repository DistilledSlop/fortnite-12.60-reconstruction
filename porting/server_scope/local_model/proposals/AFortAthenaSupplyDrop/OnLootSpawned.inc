// Behaviour snippet: AFortAthenaSupplyDrop::OnLootSpawned (body only; spliced into the generated member function).
// Original address: 0x1428347d0
// Decompile source: /opt/fortnite/FortniteClient-12.60-CL-13477524/porting/generated/native92-broad/0x1428347d0.c
//   (Ghidra flags "bad instruction data"; the .asm beside it is authoritative, 9 instructions, no calls)
// Inferred: the exec entry is P_FINISH only (shared native prologue: the counter at [rdx+0x20] is incremented if
//   nonzero: mov rax,[rdx+0x20]; xor r8d,r8d; test rax,rax; setnz r8b; add r8,rax; mov [rdx+0x20],r8), then
//   JMP 0x14191d000, the first entry of a jump table that lands at 0x14998573a, the shared epilogue stub
//   (PUSH RBP; LEA RBP,[0x241c1c3f6]; XCHG [RSP],RBP; JMP ...). No calls, no other statement. The identical
//   instruction shape is the known P_FINISH-only exec of AFortPawn::MovingEmoteStopped (0x14075b6b0, ported in
//   the checkout as an empty body). P_FINISH is the script VM epilogue of an exec thunk and is not ported (see
//   porting/decompilation_notes.md "Excerpts" and porting/behavior/AFortPlayerStateAthena/OnRep_NumChestsOpened.inc).
//   The three sibling execs of this class with the same 26-byte body (OnCheckForBeingInStorm 0x142834710,
//   OnDespawnDueToStorm 0x142834730, OnRep_NumPlayersInteracting 0x142834970) show the pattern is uniform here.
// Declared in generated header: no (generator discrepancy: the reflection record has exec 0x1428347d0 with flags
//   0x4080405, but AFortAthenaSupplyDrop.h declares only IsPlayingAReplay, OnRep_NumPlayersInteracting and
//   SetLootTableNameOverride; the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
// Empty body: the native function does nothing (see Inferred).
