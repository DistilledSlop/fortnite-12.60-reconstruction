// CONFIDENCE: high
// 0x14169c6a0, IFortTeamActorInterface byte slot 0x28.
AFortPlayerStateZone* State = Cast<AFortPlayerStateZone>(PlayerState);
return State ? State->NativeGetTeam163() : 4;
