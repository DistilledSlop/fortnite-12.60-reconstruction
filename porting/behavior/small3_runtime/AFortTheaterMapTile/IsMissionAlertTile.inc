// Original body: 0x1425702c0 (thunk 0x142b039a0)
// Offset evidence: MOV EAX,[RCX+0x430]; JS -> false; CMP EAX,[RCX+0x250]; JNZ -> false; else true
// CONFIDENCE: HIGH
return NativeRecoveredInt32_0x430 >= 0 && NativeRecoveredInt32_0x430 == TileIndex;
