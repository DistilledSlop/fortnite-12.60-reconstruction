// Original body 0x1415eefe0 (thunk 0x1428f7e70). Evidence: reflected CurrentAbilityCameraModeClass (+0xa08) must be a UFortCameraMode child; raw pointer at this+0x380 with weak at +0x18 yielding a player controller; PlayerCameraManager (+0x2b0) must be AFortPlayerCameraBase; int32 key at +0x388 (member NativeRecoveredInt32_0x388); clears the class afterwards.
// CONFIDENCE: MEDIUM
uint8* RawThis = reinterpret_cast<uint8*>(this);
if (!UFortGameplayAbility::NativeIsObjectFlagBit4Clear1406ceaa0(this))
{
	return;
}
if (CurrentAbilityCameraModeClass.Get() == nullptr)
{
	return;
}
if (!CurrentAbilityCameraModeClass.Get()->IsChildOf(UFortCameraMode::StaticClass()))
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
UFortGameplayAbility::NativeClearCameraModeForKey141ca1b30(CameraObj, &Key);
CurrentAbilityCameraModeClass = nullptr;
