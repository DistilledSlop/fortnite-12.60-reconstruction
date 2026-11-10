// CONFIDENCE: high (logic + members from reflection; engine vtable/callee identified by address + argument pattern)
// Native body at 0x1429488b0 (reflection exec entry; the leading [FFrame+0x20] counter increment is exec-entry boilerplate, not body).
// Reflected members: RopeMesh 0x218, StartLocation 0x228, WeaponMesh 0x240, WeaponMeshSocketName 0x248.
if (!RopeMesh)
{
	UE_LOG(LogFort, Warning, TEXT("AFortHookGunRope::SetStartPointOfRope: RopeMesh is nullptr, this shouldn't happen."));
	return;
}
if (!WeaponMesh)
{
	UE_LOG(LogFort, Warning, TEXT("AFortHookGunRope::SetStartPointOfRope: WeaponMesh is nullptr, this shouldn't happen."));
	return;
}
if (WeaponMeshSocketName == NAME_None)
{
	UE_LOG(LogFort, Warning, TEXT("AFortHookGunRope::SetStartPointOfRope: WeaponMeshSocketName is none, this shouldn't happen."));
	return;
}
const FVector SocketLocation = WeaponMesh->GetSocketLocation(WeaponMeshSocketName);
StartLocation = SocketLocation;
FHitResult HitResult;
RopeMesh->SetWorldLocation(SocketLocation, false, &HitResult, false);
