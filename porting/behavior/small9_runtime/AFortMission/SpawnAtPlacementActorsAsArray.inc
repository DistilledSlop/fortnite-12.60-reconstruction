// Behaviour snippet: AFortMission::SpawnAtPlacementActorsAsArray (body only; spliced into the generated function).
// Original body address: 0x1420c9fa0. Offset evidence: XOR EAX,EAX; MOV [RCX],RAX; MOV [RCX+8],RAX writes an empty TArray to the hidden return slot (RCX); MOV [R8],AL stores bSucceeded = false (third ABI argument, bool&); MOV RAX,RCX returns the slot.
// CONFIDENCE: MEDIUM
bSucceeded = false;
return TArray<AActor*>();
