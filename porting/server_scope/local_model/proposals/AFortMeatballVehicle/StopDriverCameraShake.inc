// CONFIDENCE: high (logic + members from reflection; engine vtable identified by address + argument pattern)
// Native body at 0x1429b23b0 (reflection exec entry; the leading [FFrame+0x20] counter increment is exec-entry boilerplate, not body).
// Reflected members: DrivingPlayerController 0x1070, DriverCameraShake 0x1088.
if (DrivingPlayerController && DrivingPlayerController->PlayerCameraManager)
{
	DrivingPlayerController->PlayerCameraManager->StopCameraShake(DriverCameraShake);
	DriverCameraShake = nullptr;
}
