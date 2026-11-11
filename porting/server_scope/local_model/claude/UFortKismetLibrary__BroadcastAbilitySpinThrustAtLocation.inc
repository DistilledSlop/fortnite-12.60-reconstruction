// CONFIDENCE: LOW. Same gate as the impact broadcast: Instigator must be an AFortAthenaVehicle and Radius must lie in [750, 800]. Position is read as a rotator in degrees and converted to a radian vector (Pitch, Yaw, Roll). The receiver is called with bSubtractLocalNoise = false. WorldContextObject and InSound are not read by the native body.
// Original address: 0x142967160 (exec). Native body: 0x141ee7bb0. Receiver virtuals (vslots 0x7a8, 0x600) are unnamed; see deps/UFortKismetLibrary__VehicleImpactReceiver.json.
AFortAthenaVehicle* Vehicle = Cast<AFortAthenaVehicle>(Instigator);
if (Vehicle == nullptr || Radius < 750.0f || Radius > 800.0f)
{
    return;
}
UFortVehicleImpactReceiver* Receiver = Vehicle->GetImpactReceiver();
if (Receiver == nullptr)
{
    return;
}
const FVector SpinRadians = FVector(Position.Pitch, Position.Yaw, Position.Roll) * (PI / 180.0f);
Receiver->BroadcastAbilitySpinThrustAtLocation(SpinRadians, false, Tag);
