// Original body address: 0x144f26cf0 (thunk 0x14527c9a0)
// Offset evidence: copies 0x34 bytes from object +0x1c4 to the hidden return buffer in RDX (MOVUPS x3 at +0x1c4/+0x1d4/+0x1e4, dword at +0x1f4), returns that buffer in RAX. Layout matches FFortItemViewSettings (52 bytes, ends at +0x1f8 where RegisteredInputActions starts).
// CONFIDENCE: MEDIUM
return NativeRecoveredStruct_0x1c4;
