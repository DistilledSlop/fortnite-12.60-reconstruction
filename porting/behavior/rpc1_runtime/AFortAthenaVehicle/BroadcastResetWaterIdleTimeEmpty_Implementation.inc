// Behaviour snippet: AFortAthenaVehicle::BroadcastResetWaterIdleTimeEmpty_Implementation (body only; spliced into the generated function).
// Original body address: 0x14190de50. Offset evidence: MOV dword [RCX+0xa34],0 (int32 at object offset 0xa34, no reflected member covers it; raw access).
// CONFIDENCE: HIGH
uint8* Raw = reinterpret_cast<uint8*>(this);
*reinterpret_cast<int32*>(Raw + 0xa34) = 0;
