// Original body address: 0x142706820
// Offset evidence: TEST byte ptr [RCX+0x84],0x8 is UActorComponent.bReplicates (bool_bits.json, mask 0x8 at 0x84); JNZ returns. Otherwise MOV byte ptr [RCX+0x108],DL is bMagnetized (mask 0x1 at 0x108).
// CONFIDENCE: MEDIUM
if (bReplicates) { return; }
bMagnetized = bInMagnetized;
