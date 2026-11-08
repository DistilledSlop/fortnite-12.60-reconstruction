// Original body address: 0x140695430 (thunk 0x142b99ae0)
// Offset evidence: the whole body is LEA RAX,[RCX-0x28]; RET. It makes no memory access, store, or call. It only computes this-0x28 into RAX, which is the return slot of a void implementation and is discarded.
// CONFIDENCE: HIGH
return;
