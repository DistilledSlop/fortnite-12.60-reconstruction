// CONFIDENCE: LOW. Native gate: the impact is forwarded only when Instigator is an AFortAthenaVehicle and Radius lies in [750, 800]. The vehicle's impact receiver then gets the impact at Position with bSubtractLocalNoise and Tag. WorldContextObject and InSound are not read by the native body.
// Original address: 0x142966f60 (exec). Native body: 0x141ee7af0. Receiver virtuals (vslots 0x7a8, 0x5f0) are unnamed; see deps/UFortKismetLibrary__VehicleImpactReceiver.json.
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
Receiver->BroadcastAbilityImpactAtLocation(Position, bSubtractLocalNoise, Tag);
