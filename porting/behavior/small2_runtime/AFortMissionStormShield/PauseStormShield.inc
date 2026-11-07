// Behaviour snippet: AFortMissionStormShield::PauseStormShield (body only; spliced into the generated member function).
// Original address: 0x1420e7fc0
// Evidence: Role (0xf0) compared to 3.
// Evidence: Dword load [RCX+0x2f0] (CurrentLocalRadius, Float) stored to [RCX+0x278] = RadiusGrowthData.TargetRadius (first float of FStormShieldRadiusGrowthData).
// Evidence: Byte store 0 at [RCX+0x28c] = RadiusGrowthData.State (enum at struct offset 0x14).
// CONFIDENCE: MEDIUM
if (static_cast<uint8>(Role) == 3)
{
    RadiusGrowthData.TargetRadius = CurrentLocalRadius;
    RadiusGrowthData.State = static_cast<EMissionStormShieldState>(0);
}
