// CONFIDENCE: high
// 0x142419200, IFortTeamActorInterface byte slot 0x28.
static_assert(sizeof(AFortTeamInfo) > 0, "AFortTeamInfo must be a complete type here");
return PlayerTeam ? PlayerTeam->NativeGetTeam163() : 0;
