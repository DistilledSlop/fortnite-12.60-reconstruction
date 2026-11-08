// Original body address: 0x141bc50c0 (thunk 0x142738600). Body offsets: CMP byte [RCX+0xf0],3 ; JNZ to RET ; MOV byte [RCX+0x3ed],DL ; MOV byte [RCX+0x3ec],DL ; RET.
// Offset evidence (lookup_offset.py ABuildingActor 0xf0 0x3ec 0x3ed): 0xf0 = AActor::Role (private, use GetLocalRole()), compared to 3 (ROLE_Authority).
// 0x3ed = ABuildingActor::TeamIndex (uint8), 0x3ec = ABuildingActor::Team (TEnumAsByte<EFortTeam::Type>); both stored from InTeam, TeamIndex first.
// CONFIDENCE: HIGH
if (GetLocalRole() == ROLE_Authority)
{
    TeamIndex = InTeam;
    Team = static_cast<EFortTeam::Type>(InTeam);
}
