// Original body: 0x1419d3790 (thunk 0x142a586e0). Offset evidence: [RCX+0x220] is AController::PlayerState (lookup_offset: AController +0x220 PlayerState Object; AController has no generated header, so raw access). Volume manager from AFortVolumeManager::NativeResolveVolumeManager(this); null returns null; otherwise tail call NativeFindPlayerVolume(PlayerState).
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
AFortVolumeManager* ResolvedManager = AFortVolumeManager::NativeResolveVolumeManager(const_cast<AFortPlayerControllerAthena*>(this));
if (ResolvedManager == nullptr)
{
	return nullptr;
}
APlayerState* ControllerPlayerState = *reinterpret_cast<APlayerState* const*>(Raw + 0x220);
return ResolvedManager->NativeFindPlayerVolume(ControllerPlayerState);
