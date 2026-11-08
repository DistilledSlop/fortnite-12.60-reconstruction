// Original body address: 0x142681940 (thunk 0x1428c4a10)
// Offset evidence: MOV byte ptr [RCX+0x118c],DL; RET. 0x118c (4492) is the bool bitfield bAlwaysMoveFreely (bool_bits.json, mask 1). DL = bNewValue.
// CONFIDENCE: HIGH
bAlwaysMoveFreely = bNewValue;
