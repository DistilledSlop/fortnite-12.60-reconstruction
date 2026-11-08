// Behaviour snippet: AFortMinigame::IsPlayerInOurVolume (body 0x14209a4e0, thunk 0x1429c3560).
// Offset evidence: Volume (reflected Object, +0x910) is passed as the volume-manager context and as the third volume argument (this_delta 0).
// Asm: VM = NativeResolveVolumeManager(Volume); if null return false; return NativeVolumeContainsUserAt288141c94c20(VM, PlayerState, Volume).
// CONFIDENCE: HIGH
static_assert(sizeof(AFortVolume) > 0, "AFortVolume must be complete for the Volume argument conversion");
static_assert(sizeof(AFortVolumeManager) > 0, "AFortVolumeManager must be complete for the helper return");
AFortVolumeManager* VolumeManager = AFortVolumeManager::NativeResolveVolumeManager(Volume);
if (VolumeManager == nullptr)
{
    return false;
}
return AFortMinigame::NativeVolumeContainsUserAt288141c94c20(VolumeManager, PlayerState, Volume);
