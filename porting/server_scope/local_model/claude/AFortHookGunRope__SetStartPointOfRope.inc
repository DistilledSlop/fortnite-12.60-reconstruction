// CONFIDENCE: HIGH
// Original address: 0x1429488b0 (exec, native body; no FFrame parameters). Virtual 0x410 on WeaponMesh = USceneComponent::GetSocketLocation (captured 0x1441167b0). Helper 0x144124390 = USceneComponent::SetWorldLocation(NewLocation, bSweep=false, OutHit=nullptr, Teleport=None).
if (RopeMesh == nullptr)
{
    // Original gates the log on the category verbosity; both paths return.
    UE_LOG(LogFortniteGame, Warning, TEXT("AFortHookGunRope::SetStartPointOfRope: RopeMesh is nullptr, this shouldn't happen."));
    return;
}
if (WeaponMesh == nullptr)
{
    UE_LOG(LogFortniteGame, Warning, TEXT("AFortHookGunRope::SetStartPointOfRope: WeaponMesh is nullptr, this shouldn't happen."));
    return;
}
if (WeaponMeshSocketName.IsNone())
{
    UE_LOG(LogFortniteGame, Warning, TEXT("AFortHookGunRope::SetStartPointOfRope: WeaponMeshSocketName is none, this shouldn't happen."));
    return;
}
StartLocation = WeaponMesh->GetSocketLocation(WeaponMeshSocketName);
RopeMesh->SetWorldLocation(StartLocation, false, nullptr, ETeleportType::None);
