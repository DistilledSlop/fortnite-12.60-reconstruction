// CONFIDENCE: MEDIUM. Initializes a building spawned from Kismet: picks the build-placement animation state (player build versus default), passes the spawning controller's player id, then notifies the state change.
// Original address: 0x142737640 (exec). Native body: 0x141baf090 (virtual slot 0x810). The states 1 and 4 are the decompiled constants; their enum names are unverified.
const uint16 PlayerId = SpawningController ? GetPlayerIdForController(SpawningController) : 0xFFFF;

if (bUsePlayerBuildAnimations)
{
    SetBuildingAnimationState(EBuildingAnimationState::PlayerBuild);
    NotifyBuildingStateChanged(EBuildingAnimationState::PlayerBuild);
}
else
{
    SetBuildingAnimationState(EBuildingAnimationState::Spawned, PlayerId, BuildingOwner);
    NotifyBuildingStateChanged(EBuildingAnimationState::Spawned);
}
