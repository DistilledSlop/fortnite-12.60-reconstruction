// Original body 0x14160c620 (thunk 0x1428f9580). Evidence: same guard chain as ClearCameraMode (flag bit, raw owner pointer at +0x380 with weak +0x18, controller IsA, PlayerCameraManager IsA AFortPlayerCameraBase), key int32 at +0x388 (member NativeRecoveredInt32_0x388); the class is stored only on the success path.
// CONFIDENCE: MEDIUM
uint8* RawThis = reinterpret_cast<uint8*>(this);
if (!UFortGameplayAbility::NativeIsObjectFlagBit4Clear1406ceaa0(this))
{
	return;
}
uint8* OwnerRaw = *reinterpret_cast<uint8**>(RawThis + 0x380);
if (OwnerRaw == nullptr)
{
	return;
}
UObject* ControllerObj = reinterpret_cast<const TWeakObjectPtr<UObject>*>(OwnerRaw + 0x18)->Get();
if (ControllerObj == nullptr)
{
	return;
}
if (!ControllerObj->IsA(AFortPlayerController::StaticClass()))
{
	return;
}
AFortPlayerController* PlayerController = reinterpret_cast<AFortPlayerController*>(ControllerObj);
UObject* CameraObj = PlayerController->PlayerCameraManager;
if (CameraObj == nullptr)
{
	return;
}
if (!CameraObj->IsA(AFortPlayerCameraBase::StaticClass()))
{
	return;
}
const int32 Key = NativeRecoveredInt32_0x388;
UFortGameplayAbility::NativeSetCameraModeClassForKey141cb7690(CameraObj, CameraModeClass.Get(), &Key);
CurrentAbilityCameraModeClass = CameraModeClass;
