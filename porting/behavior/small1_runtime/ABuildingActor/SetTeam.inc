// Original address: 0x141bc50c0 (ABuildingActor::SetTeam)
// Offset evidence: [RCX+0xf0] is AActor Role (Byte); [RCX+0x3ed] is TeamIndex and [RCX+0x3ec] is Team (both Byte, reflected on ABuildingActor).
// CONFIDENCE: HIGH
if (static_cast<uint8>(GetLocalRole()) != 3)
{
    return;
}
TeamIndex = InTeam;
Team = InTeam;
