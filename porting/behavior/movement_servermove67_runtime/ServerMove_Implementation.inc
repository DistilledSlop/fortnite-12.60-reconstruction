// Original address: 0x1420ed7e0.
// CONFIDENCE: HIGH
// Complete Fortnite ServerMove receive-side water time scope.
UWaterSubsystem* WaterSubsystem = UWaterSubsystem::GetWaterSubsystem(GetWorld());
float PreviousOverrideTime = 0.0f;
bool bPreviouslyOverridingTime = false;
if (WaterSubsystem)
{
    PreviousOverrideTime = WaterSubsystem->GetOverrideSmoothedWorldTimeSeconds();
    bPreviouslyOverridingTime = WaterSubsystem->GetShouldOverrideSmoothedWorldTimeSeconds();
    WaterSubsystem->SetShouldOverrideSmoothedWorldTimeSeconds(
        GFortWaterUseClientWaveTime67 != 0);
}

if (InAccel.Z >= 90000.0f)
{
    if (WaterSubsystem)
    {
        WaterSubsystem->SetOverrideSmoothedWorldTimeSeconds(
            (InAccel.Z - 90000.0f) * 0.099999994f);
    }
    InAccel.Z = 0.0f;
}

UCharacterMovementComponent::ServerMove_Implementation(
    TimeStamp, InAccel, ClientLoc, CompressedMoveFlags, ClientRoll, View,
    ClientMovementBase, ClientBaseBoneName, ClientMovementMode);

if (WaterSubsystem)
{
    WaterSubsystem->SetOverrideSmoothedWorldTimeSeconds(PreviousOverrideTime);
    WaterSubsystem->SetShouldOverrideSmoothedWorldTimeSeconds(bPreviouslyOverridingTime);
}
