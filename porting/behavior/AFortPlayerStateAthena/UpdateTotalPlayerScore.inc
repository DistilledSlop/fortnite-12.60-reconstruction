// Behaviour snippet: AFortPlayerStateAthena::UpdateTotalPlayerScore (kind helper: a non-reflected member; body only).
// Original address: 0x141a3c390 (queue row 3 of porting/behavior/CALLEE_PORT_QUEUE.md).
// Decompile source: porting/generated/callee_probe/blocking/0x141a3c390.c (disassembly in the .asm beside it)
// Proxy score: none (queue row 3; not a portability record)
// Inferred: the new total is the sum of ten int32 point caches: KillScorePoints (+0xe88) and the nine caches of the
//   stats OnRep_Num* write (+0xebc to +0xefc). AIKillCountPoints (+0xeb4) is not in the sum (the decompile skips it).
//   If the role is ROLE_Authority (GetLocalRole() returns the AActor Role member, which is private in UE 4.25) and the total is larger than TotalPlayerScore, the difference is added to the
//   AFortTeamInfo of PlayerTeam (the vfn at +0x770 returns PlayerTeam: AFortPlayerState::GetPlayerTeam at 0x1424192a0,
//   asm mov rax,[rcx+0x640]; ret; read directly here) when that team IsA AFortTeamInfoAthena (+0x370, the field
//   AccumulatedPlayerScore). TotalPlayerScore (+0xf00) is then set to the new total. When the role is not ROLE_Authority,
//   the native list TotalPlayerScoreNative (+0x1180) is broadcast with (this, total). The decompile broadcasts in that
//   branch only, so an authority never broadcasts here, whatever the delta.
// Declared in generated header: no (helper: the generator declares it, see porting/behavior/README.md, "helper")
// CONFIDENCE: HIGH
const int32 NewTotal = KillScorePoints
	+ NumChestsOpenedPoints
	+ NumAmmoCansOpenedPoints
	+ NumSupplyDropsOpenedPoints
	+ NumLlamasOpenedPoints
	+ NumForagedItemsConsumedPoints
	+ NumMinutesAlivePoints
	+ NumBronzeCoinsCollectedPoints
	+ NumSilverCoinsCollectedPoints
	+ NumGoldCoinsCollectedPoints;
if (GetLocalRole() == ROLE_Authority)
{
	const int32 Delta = NewTotal - TotalPlayerScore;
	if (Delta > 0)
	{
		AFortTeamInfo* Team = PlayerTeam;
		if (Team != nullptr && Team->IsA(AFortTeamInfoAthena::StaticClass()))
		{
			Team->AccumulatedPlayerScore += Delta;
		}
	}
}
TotalPlayerScore = NewTotal;
if (GetLocalRole() != ROLE_Authority)
{
	TotalPlayerScoreNative.Broadcast(this, NewTotal);
}
