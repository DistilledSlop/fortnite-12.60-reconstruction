// Helper 0x141c94c20 on AFortMinigame (callers IsPlayerInOurVolume 0x14209a4e0 and 3 others).
// Asm: ADD RCX,0x288; JMP 0x141c6ad60 (tail call to VolumeContainsUser, registers RDX/R8 pass through unchanged).
// Return value is AL from the callee; all instructions accounted for.
// CONFIDENCE: MEDIUM
AFortVolumeManager* Manager = reinterpret_cast<AFortVolumeManager*>(reinterpret_cast<uint8*>(Context) + 0x288);
return Manager->VolumeContainsUser(Volume, PlayerState);
