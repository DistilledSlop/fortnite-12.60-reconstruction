// Original body: 0x141a48210 (thunk 0x142ad2440)
// Offset evidence: MOV RAX,[RCX+0x3a0]; MOVUPS XMM1,[RAX+0x1d0]; store x,y,z to hidden FVector return
// CONFIDENCE: MEDIUM
const float* Center = reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(SafeZoneMesh) + 0x1d0);
return FVector(Center[0], Center[1], Center[2]);
