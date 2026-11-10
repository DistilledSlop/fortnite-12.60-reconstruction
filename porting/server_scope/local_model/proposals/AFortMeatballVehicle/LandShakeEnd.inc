// CONFIDENCE: high (logic + members from reflection; engine vtable/callee identified by address + argument pattern)
// Native body at 0x1429b2390 (reflection exec entry; the leading [FFrame+0x20] counter increment is exec-entry boilerplate, not body).
// Reflected members: DrivingPlayerController 0x1070, LandCameraShake 0x1078, LandForceFeedbackHandle 0x10a0.
if (LandCameraShake && DrivingPlayerController)
{
	if (DrivingPlayerController->PlayerCameraManager)
	{
		DrivingPlayerController->PlayerCameraManager->StopCameraShake(LandCameraShake);
	}
	LandCameraShake = nullptr;
	// Native re-reads DrivingPlayerController after StopCameraShake before stopping the rumble.
	if (DrivingPlayerController)
	{
		DrivingPlayerController->StopForceFeedback(2, LandForceFeedbackHandle);
	}
}
