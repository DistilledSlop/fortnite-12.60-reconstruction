// Behaviour snippet: AFortPlayerPawn::OnRep_ReplicatedIsInSlipperyMovement (body only; spliced into the generated member function).
// Original address: 0x142a7e140
// Decompile source: porting/generated/decomp/AFortPlayerPawn/OnRep_ReplicatedIsInSlipperyMovement.c
// Proxy score: 92.9 (portability.json)
// Inferred: Decompile: if (byte +0x10e8 bit 0x1) != (byte +0x10e7 bit 0x80), the 0x80 bit of +0x10e7 is set to the 0x1 bit of +0x10e8 (the shift-by-7 store). Bits: bReplicatedIsInSlipperyMovement (0x10e8, 0x1), bIsInSlipperyMovement (0x10e7, 0x80).
// Declared in generated header: yes
// CONFIDENCE: MEDIUM
if (bReplicatedIsInSlipperyMovement != bIsInSlipperyMovement)
{
    bIsInSlipperyMovement = bReplicatedIsInSlipperyMovement;
}
