// Behaviour snippet: AFortAthenaVehicle::ServerSetIgnoreAllFallingDamage_Implementation (body only; spliced into the generated function).
// Original body address: 0x141920590. Offset evidence: AND byte [RCX+0x40c],0x7f; SHL DL,7; OR byte [RCX+0x40c],DL = declared bitfield bIgnoreAllFallingDamage (mask 0x80 at 0x40c, bool_bits.json).
// CONFIDENCE: HIGH
bIgnoreAllFallingDamage = InIgnoreAllFallingDamage;
