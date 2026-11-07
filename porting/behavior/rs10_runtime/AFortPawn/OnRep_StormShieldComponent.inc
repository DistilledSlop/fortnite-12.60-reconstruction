// Behaviour snippet: AFortPawn::OnRep_StormShieldComponent (body only; spliced into the generated member function).
// Original body address: 0x14233dc60 (thunk 0x142a1eca0, group B, flags 0x80401).
// Offset evidence: this+0x558 = StormShieldComponent (UFortStormShieldComponent*, reflected); this+0xf0 = Role (AActor, Byte, reflected, 3 = ROLE_Authority); this+0x560 = OnStormShieldComponentCreated (reflected multicast delegate, one UFortStormShieldComponent* parameter).
// Pointee offsets +0x3a (OR bit 0x2) and +0x3c (dword 0x3e800000 = 0.25f) lie in UFortStormShieldComponent engine storage with no reflected member, so they are raw accesses.
// CONFIDENCE: MEDIUM
if (StormShieldComponent == nullptr)
{
    return;
}
if (GetLocalRole() != ROLE_Authority)
{
    uint8* ShieldRaw = reinterpret_cast<uint8*>(StormShieldComponent);
    ShieldRaw[0x3a] |= 0x2;
    *reinterpret_cast<uint32*>(ShieldRaw + 0x3c) = 0x3e800000u;
}
OnStormShieldComponentCreated.Broadcast(StormShieldComponent);
