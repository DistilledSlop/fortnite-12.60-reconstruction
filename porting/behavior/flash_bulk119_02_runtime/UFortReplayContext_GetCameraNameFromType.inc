// CONFIDENCE: high
// Native body at 0x142469050 switches on CamType to return localized camera name.
switch (CamType)
{
case ESpectatorCameraType::ThirdPerson:
    return NSLOCTEXT("ReplayContext", "ThirdPersonCamName", "Third Person");
case ESpectatorCameraType::DroneFree:
    return NSLOCTEXT("ReplayContext", "DroneFreeCamName", "Drone Free");
case ESpectatorCameraType::Gameplay:
    return NSLOCTEXT("ReplayContext", "GameplayCamName", "Gameplay");
case ESpectatorCameraType::ReverseShot:
    return NSLOCTEXT("ReplayContext", "ReverseShotCamName", "Reverse Shot");
case ESpectatorCameraType::FollowShot:
    return NSLOCTEXT("ReplayContext", "FollowShotCamName", "Follow Shot");
case ESpectatorCameraType::DroneFollow:
    return NSLOCTEXT("ReplayContext", "DroneFollowCamName", "Drone Follow");
case ESpectatorCameraType::DroneAttach:
    return NSLOCTEXT("ReplayContext", "DroneAttachCamName", "Drone Attach");
case ESpectatorCameraType::BattleMap:
    return NSLOCTEXT("ReplayContext", "BattleMapCamName", "Battle Map");
default:
    return NSLOCTEXT("ReplayContext", "UnknownCamName", "Unknown");
}
