// Helper 0x1418d31e0: setter for AAthenaCapturePoint::TeamControllingPoint (+0xab9) that also refreshes
// TeamInfoControllingPoint (+0xac0). Evidence:
//  - vtbl[0x150] on the actor targets 0x143f793a0 (reads [this+8] flags, outer walk) = AActor::GetWorld; [World+0x130] = UWorld::GameState
//    (lookup_offset UWorld 0x130).
//  - 0x14290f690 = AFortGameStateAthena::StaticClass() (hinted); 0x142af1340 = AFortTeamInfoAthena::StaticClass().
//  - vtbl[0x880] on AFortGameState targets 0x141e9a7f0 = `LEA RAX,[RCX+0x470]; RET` and lookup_offset AFortGameState 0x470 = Teams.
//  - [Team+0x218] is the IFortTeamActorInterface sub-object (AFortTeamInfo::Team @0x220 follows); interface slot 0x28 is
//    AFortTeamInfo::NativeGetTeam163 (UFortTeamActorInterface slot 0x28 = 0x142ec4e20, AFortTeamInfo.cpp documents slot 0x28).
if (CapturePoint->TeamControllingPoint == NewTeamIndex)
{
	return;
}
CapturePoint->TeamControllingPoint = NewTeamIndex;
if (NewTeamIndex == 0)
{
	CapturePoint->TeamInfoControllingPoint = nullptr;
	return;
}
const UWorld* OwningWorld = CapturePoint->GetWorld();
AFortGameStateAthena* AthenaState = (OwningWorld != nullptr) ? Cast<AFortGameStateAthena>(OwningWorld->GetGameState()) : nullptr;
if (AthenaState == nullptr)
{
	return;
}
for (AFortTeamInfo* TeamEntry : AthenaState->Teams)
{
	if (TeamEntry != nullptr && TeamEntry->NativeGetTeam163() == CapturePoint->TeamControllingPoint)
	{
		// Original keeps the matching entry only when it is an AFortTeamInfoAthena (inline class-chain check), else stores null.
		CapturePoint->TeamInfoControllingPoint = Cast<AFortTeamInfoAthena>(TeamEntry);
		return;
	}
}
// CONFIDENCE: MEDIUM
