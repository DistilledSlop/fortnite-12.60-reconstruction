// Original body address: 0x1418f7280 (thunk 0x142831990, RPC implementation)
// Offset evidence: this+0x104c is the reflected bool bPlayerTorquingRepped (bool_bits mask 1 at 0x104c); the body stores the bSetTo argument (DL) into that byte.
// CONFIDENCE: HIGH
bPlayerTorquingRepped = bSetTo;
