// Behaviour snippet: UFortKismetLibrary::HasRewards (body only; spliced into the generated static function).
// Original address: body 0x141ef7ee0 (thunk 0x142971bd0)
// Offset evidence: body is `XOR EDX,EDX; JMP 0x141f8e580`. RCX (the FFortRewardInfo reference) passes through unchanged as the Rewards pointer; EDX=0 sets bIncludeExtended=false. No memory access in this body.
// Callee: NativeHasAnyRewardEntries141f8e580 (ported helper, declared static in porting/native/Source/FortniteGame/Generated/UFortKismetLibrary.h).
// CONFIDENCE: HIGH
return NativeHasAnyRewardEntries141f8e580(&RewardInfo, false);
