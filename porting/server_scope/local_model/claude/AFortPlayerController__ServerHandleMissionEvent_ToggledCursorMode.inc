// CONFIDENCE: MEDIUM. Original exec 0x142a4b8b0 (RPC thunk, one bool parameter bOpened).
// Validate is virtual slot 0xec8 -> Stub_ReturnTrue (0x1406801d0), so no validation runs.
// Implementation is virtual slot 0xed0 -> 0x1423ab600 (124 lines), inlined below.
// Gate: only when this controller's role byte (+0xf0) is ROLE_Authority (3).
if (GetLocalRoleByte() == ROLE_Authority)
{
	UWorld* World = GetWorld();
	UFortMissionManager* MissionManager = GetZoneMissionManager_0x1420b7ab0(World);
	if (MissionManager != nullptr)
	{
		UFortToggledCursorModeParams* Params = NewObject<UFortToggledCursorModeParams>(GetTransientPackage_0x142e80820());
		Params->Controller_0x28 = this;
		Params->bOpened_0x30 = bOpened;
		FFortMissionEventData Data;
		Data.Tag = MissionEventTag_0x14813fb18;
		Data.Context = GetMissionEventContext_0x1407c2c80();
		Data.Payload = Params;
		MissionManager->HandleMissionEvent_Slot0x638(Data);
	}
}
