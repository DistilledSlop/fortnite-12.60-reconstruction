// Behaviour snippet: UFortMissionLibrary::FindActorsWithTags (body only; spliced into the generated function).
// Original body address: 0x140738030. Offset evidence: XOR EAX,EAX; MOV [RCX],RAX; MOV [RCX+8],RAX writes an empty TArray to the hidden return slot (RCX). RDX and R8 are not read.
// CONFIDENCE: MEDIUM
return TArray<ABuildingActor*>();
