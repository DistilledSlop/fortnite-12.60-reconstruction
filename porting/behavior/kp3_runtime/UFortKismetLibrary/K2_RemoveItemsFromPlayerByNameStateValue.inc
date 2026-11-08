// Behaviour snippet: UFortKismetLibrary::K2_RemoveItemsFromPlayerByNameStateValue (body only).
// Original address: 0x141efa6c0 (thunk 0x142973b80; body shared with K2_RemoveItemsFromPlayerByIntStateValue)
// Offset evidence: no member access. The body tail-jumps to 0x140680300, which is XOR EAX,EAX; RET (constant 0, no effect). Every path returns 0, so the tail jump is folded.
// CONFIDENCE: HIGH
return 0;
