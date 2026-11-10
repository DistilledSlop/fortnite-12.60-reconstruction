// CONFIDENCE: MEDIUM; thunk 0x142a7f2f0 has no validate and calls impl 0x14234a360 directly; the CanBeHoistedBy check is vcall 0x11c8 on the hoistee. Impl: if hoistee non-null and this->Controller IsA AFortPlayerController and hoistee->CanBeHoistedBy(Controller) (vcall 0x11c8 on hoistee), record the hoist (hoistee.DBNOHoister=this, hoistee+0x3ea=false, this.DBNOHoistee=hoistee), bump the hoist counter on the hoistee's PlayerState (+0x952 same team, +0x954 otherwise), store the hoist location, reset hoist state via 0x142336990 (406 lines, only read in part; not ported to depth two), then refresh via vcall 0x5e8.
if (InDBNOHoistee && Controller && Controller->IsA(AFortPlayerController::StaticClass()) && InDBNOHoistee->CanBeHoistedBy(Controller))
{
	InDBNOHoistee->SetDBNOHoister(this, /*bIsInterrogating=*/false);
	DBNOHoistee = InDBNOHoistee;
	AFortPlayerStateAthena* HoisteePS = Cast<AFortPlayerState>(InDBNOHoistee->PlayerState);
	if (HoisteePS)
	{
		if (InDBNOHoistee->GetTeamIndex() == GetTeamIndex())   // vcall 0x28 on embedded objects at +0x4a8
		{
			HoisteePS->NativeRecovered_0x952 += 1;
		}
		else
		{
			HoisteePS->NativeRecovered_0x954 += 1;
		}
		HoisteePS->NativeRecovered_0x964 = HoistRootLocationOrDefault(InDBNOHoistee);  // stores FVector at +0x964 and +0x96c   // from hoistee root component +0x1d0 or constant
	}
	ResetDBNOHoistState(InDBNOHoistee, 0, 0);   // 0x142336990
	InDBNOHoistee->RefreshReplicatedState();   // vcall 0x5e8
}
