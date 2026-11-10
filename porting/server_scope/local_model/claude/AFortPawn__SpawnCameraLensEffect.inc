// CONFIDENCE: MEDIUM
// Original: exec thunk 0x142a20700; native body 0x142351740.
// Helpers: 0x142321a70 (FindPlayerControllerForPawn, ported in deps), 0x142a35cb0 (StaticClass FortPlayerCameraBase, engine-style).
// The lens-effect object itself is produced by the camera's virtual at slot 0x770 (NOT PORTED, see evidence notes).
UObject* PlayerController = FindPlayerControllerForPawn(this);
if (!PlayerController || !PlayerController->GetClass()->IsChildOf(AFortPlayerController::StaticClass()))
{
    return nullptr;
}

AFortPlayerController* FortPC = static_cast<AFortPlayerController*>(PlayerController);
AFortPlayerCameraBase* CameraBase = Cast<AFortPlayerCameraBase>(FortPC->PlayerCameraManager); // +0x2b0
if (!CameraBase)
{
    return nullptr;
}

return CameraBase->NativeSpawnCameraLensEffect(LensEffectEmitterClass); // vslot 0x770, target unnamed
