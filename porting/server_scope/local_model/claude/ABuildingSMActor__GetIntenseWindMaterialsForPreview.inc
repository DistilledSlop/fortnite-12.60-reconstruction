// CONFIDENCE: HIGH
// Original address: 0x14273fe60 (exec thunk). Its Ghidra capture label reads AFortPlayerControllerAthena_HasItems, but the body is the same shared stub.
// The thunk steps the Materials out-array into a temporary, calls Stub_ReturnFalse (0x1406801e0: xor eax,eax; ret) and frees the temporary.
// Materials is never written back, so the caller array is left unchanged. The true body returns a constant false.
return false;
