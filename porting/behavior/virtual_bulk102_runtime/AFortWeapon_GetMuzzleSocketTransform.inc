// Native target 0x14269dc10; confidence: high.
if (WeaponMesh && WeaponMesh->SkeletalMesh)
{
    return WeaponMesh->GetSocketTransform(GetMuzzleSocketName(PatternIndex), RTS_World);
}
return GetActorTransform();
