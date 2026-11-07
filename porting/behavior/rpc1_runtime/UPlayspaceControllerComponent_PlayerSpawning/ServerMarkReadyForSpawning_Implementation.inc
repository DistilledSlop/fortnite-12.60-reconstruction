// Behaviour snippet: UPlayspaceControllerComponent_PlayerSpawning::ServerMarkReadyForSpawning_Implementation (body only; spliced into the generated function).
// Original body address: 0x14076c610. Offset evidence: MOV byte [RCX+0xc8],1; offset 0xc8 follows SpawnCameraActor (Object at 0xc0, ends 0xc8) and no reflected member covers it; raw access.
// CONFIDENCE: HIGH
uint8* Raw = reinterpret_cast<uint8*>(this);
*reinterpret_cast<uint8*>(Raw + 0xc8) = 1;
