// Original body 0x141a7c960 (thunk 0x142b31d50). Offset evidence: pointer at +0xa0 (NativeRecoveredUint64_0xa0; gap between UCSModifiedProperties at +0x90 and MuzzleSocketNames at +0xb0). IsA(AActor::StaticClass()) via call 0x1446acb00 is Cast<AActor>.
// CONFIDENCE: HIGH
UObject* Candidate = reinterpret_cast<UObject*>(NativeRecoveredUint64_0xa0);
return Cast<AActor>(Candidate);
