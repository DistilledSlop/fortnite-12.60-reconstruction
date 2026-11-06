// CONFIDENCE: high
// 0x1423d6280. Resolve the virtual state even when role/camera rules reject it.
AFortPlayerState* State = NativeGetSpectatingPlayerState163();
if (GetLocalRole() == ROLE_Authority || static_cast<uint8>(CurrentCameraType) != 2 || !State)
{
    return 0;
}
return State->NativeGetTeam163();
