// Original body address: 0x1406a7c40 (group C thunk 0x14080bc70)
// Offset evidence: body is only `LEA RAX,[RCX + -0x108]; RET`. The LEA result is the return register of a void function and is discarded; no loads or stores.
// CONFIDENCE: MEDIUM
return;
