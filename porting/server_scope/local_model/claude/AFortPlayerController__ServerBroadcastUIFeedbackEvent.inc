// CONFIDENCE: MEDIUM - dispatch structure is fully read; the per-controller send helper 0x1423bd380 (70 lines) is only partly ported (see deps).
// Original address: 0x142a4a410 (RPC exec thunk, EventName Name). Validate is virtual slot 0x1208 -> Stub_ReturnTrue (0x1406801d0).
// Implementation is virtual slot 0x1210 -> 0x14236d0e0.
if (!ServerBroadcastUIFeedbackEvent_Validate(EventName))
{
	RPC_ValidateFailed(TEXT("ServerBroadcastUIFeedbackEvent_Validate"));
	return;
}
if (GetNetMode() == ENetMode::NM_Client)
{
	// Client: run the event locally on this controller (helper 0x142a3e640; the parameter block is the bare FName).
	ProcessEvent(FindFunctionChecked(FName_0x1481af0f0), &EventName);
	return;
}
UWorld* World = GetWorld();
for (const TWeakObjectPtr<UObject>& Entry : World->PlayerControllerList_0x1c8)
{
	AFortPlayerController* Controller = Cast<AFortPlayerController>(Entry.Get());
	if (Controller != nullptr)
	{
		SendUIFeedbackToController_0x1423bd380(Controller, EventName);
	}
}
