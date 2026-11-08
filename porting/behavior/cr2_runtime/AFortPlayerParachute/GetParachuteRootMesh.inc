// Original body: 0x141a15100 (thunk 0x142a7c4f0). Offset evidence: [RCX+0x348] is AFortPlayerParachute::ParachuteMesh (reflected Object, offset 0x348); the pointer value is returned unchanged.
// CONFIDENCE: HIGH
return reinterpret_cast<USkeletalMeshComponent*>(ParachuteMesh);
