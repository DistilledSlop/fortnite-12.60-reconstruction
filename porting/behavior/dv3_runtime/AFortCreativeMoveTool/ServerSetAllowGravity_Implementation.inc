// Original body address: 0x142681930 (thunk 0x1428c4950)
// Offset evidence: MOV byte ptr [RCX+0x1119],DL; RET. 0x1119 (4377) is the bool bitfield bAllowGravityOnPlace (bool_bits.json, mask 1, only declared bit in that byte). DL = bAllow.
// CONFIDENCE: HIGH
bAllowGravityOnPlace = bAllow;
