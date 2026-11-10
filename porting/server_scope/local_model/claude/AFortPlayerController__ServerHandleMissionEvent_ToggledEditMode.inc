// CONFIDENCE: MEDIUM. Original exec 0x142a4b970 (RPC thunk). Parameters in read order: EditableActor (object), bOpened (bool).
// Validate is virtual slot 0xed8 -> Stub_ReturnTrue (0x1406801d0), so no validation runs.
// Implementation is virtual slot 0xee0 -> 0x1423ab820 (124 lines), inlined below.
// Gate: only when this controller's role byte (+0xf0) is ROLE_Authority (3).
if (GetLocalRoleByte() == ROLE_Authority)
{
	UWorld* World = GetWorld();
	UFortMissionManager* MissionManager = GetZoneMissionManager_0x1420b7ab0(World);
	if (MissionManager != nullptr)
	{
		UFortToggledEditModeParams* Params = NewObject<UFortToggledEditModeParams>(GetTransientPackage_0x142e80820());
		Params->Controller_0x28 = this;
		Params->EditableActor_0x30 = EditableActor;
		Params->bOpened_0x38 = bOpened;
		FFortMissionEventData Data;
		Data.Tag = MissionEventTag_0x14813fb28;
		Data.Context = GetMissionEventContext_0x1407c2c80();
		Data.Payload = Params;
		MissionManager->HandleMissionEvent_Slot0x638(Data);
	}
}
