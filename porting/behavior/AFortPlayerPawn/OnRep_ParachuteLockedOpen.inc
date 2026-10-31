// Behaviour snippet: AFortPlayerPawn::OnRep_ParachuteLockedOpen (body only; spliced into the generated member function).
// Original address: 0x142a7e080
// Decompile source: porting/generated/decomp/AFortPlayerPawn/OnRep_ParachuteLockedOpen.c
// Proxy score: 92.5 (portability.json)
// Inferred: Decompile copies whole byte +0x2284 to +0x2285. bool_bits.py maps +0x2284 to bParachuteLockedOpen and +0x2285 to bLocalParachuteLockedOpen (both mov8, mask 1); the copy is taken as the flag copy.
// Declared in generated header: yes
// CONFIDENCE: MEDIUM
bLocalParachuteLockedOpen = bParachuteLockedOpen;
