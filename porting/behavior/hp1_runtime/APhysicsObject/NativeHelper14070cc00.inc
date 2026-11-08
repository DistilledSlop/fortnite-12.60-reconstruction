// Helper 0x14070cc00, host APhysicsObject (static).
// Evidence: MOVSS XMM0,dword ptr [RCX+0xc]; RET. Context is an interior struct pointer, not a UObject start.
// Callers ABuildingActor::GetHealth and GetMaxHealth pass [this+0x318]+0x30 and [this+0x318]+0x58 and return the float directly.
// CONFIDENCE: MEDIUM
return *reinterpret_cast<const float*>(Context + 0xc);
