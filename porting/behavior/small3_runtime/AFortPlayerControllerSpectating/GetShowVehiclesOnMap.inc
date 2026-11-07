// Original body: 0x1423d5a50 (thunk 0x142a6bae0)
// Offset evidence: CMP byte [RCX+0x2ae6],0 and CMP byte [RCX+0x2ae5],0; both nonzero -> true, else false
// CONFIDENCE: HIGH
return NativeRecoveredBool_0x2ae6 && NativeRecoveredBool_0x2ae5;
