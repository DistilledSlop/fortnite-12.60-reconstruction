// Original body address: 0x141aaf8e0 (thunk 0x1427e94c0); the same body is shared by AFortAthenaMutator_Bagel and AFortAthenaMutator_Mash.
// Offset evidence: RiftDamagerInfo = reflected TArray<FRiftDamagerInfo> at this+0x12d0 (Num +0x12d8). FRiftDamagerInfo is 24 bytes: Rift (ABuildingRift*, +0) and Damagers (TArray<AController*>, +8).
// ABI: RDX = DamagedActor, XMM2 = Damage (never read by the body), R9 = InstigatedBy (AController*). The remaining arguments are never read.
// Search for an element whose Rift equals DamagedActor and AddUnique InstigatedBy to its Damagers; otherwise append a zeroed element with Rift = Cast<ABuildingRift>(DamagedActor) and Damagers = {InstigatedBy}.
// CONFIDENCE: MEDIUM
static_assert(sizeof(ABuildingRift) > 0, "ABuildingRift must be a complete type for Cast<ABuildingRift>");
(void)Damage;
(void)HitLocation;
(void)FHitComponent;
(void)BoneName;
(void)Momentum;
(void)DamageCauser;
const int32 ElementCount = RiftDamagerInfo.Num();
for (int32 Index = 0; Index < ElementCount; ++Index)
{
    FRiftDamagerInfo& Existing = RiftDamagerInfo[Index];
    if (Existing.Rift == DamagedActor)
    {
        Existing.Damagers.AddUnique(InstigatedBy);
        return;
    }
}
RiftDamagerInfo.AddDefaulted();
FRiftDamagerInfo& NewInfo = RiftDamagerInfo.Last();
NewInfo.Rift = Cast<ABuildingRift>(DamagedActor);
NewInfo.Damagers.Add(InstigatedBy);
