// Native 0x1415f5e00. CONFIDENCE: high
OutBone = NAME_None;
OutPoint = FVector::ZeroVector;
USkeletalMeshComponent* Mesh = Pawn->GetMesh();
if (Mesh)
{
    float ClosestDistanceSquared = MAX_flt;
    for (int32 ZoneIndex = 0; ZoneIndex < 4; ++ZoneIndex)
    {
        if ((!bRestrictZone || ZoneIndex == RestrictedZone) && Pawn->DamageZones[ZoneIndex].bActive)
        {
            for (FName Bone : Pawn->DamageZones[ZoneIndex].Bones)
            {
                const int32 BoneIndex = Mesh->GetBoneIndex(Bone);
                if (BoneIndex == INDEX_NONE)
                {
                    continue;
                }
                const FMatrix BoneMatrix = Mesh->GetBoneMatrix(BoneIndex);
                const FVector Center = BoneMatrix.TransformPosition(Offset);
                const float Radius = BoneMatrix.GetScaleVector(SMALL_NUMBER).GetMax() * SphereRadius;
                const FVector ToCenter = Center - Start;
                const float Projection = FVector::DotProduct(ToCenter, Direction);
                const float Discriminant = Radius * Radius - (ToCenter.SizeSquared() - Projection * Projection);
                if (Discriminant < 0.f)
                {
                    continue;
                }
                const float Time = (Projection - FMath::Sqrt(Discriminant)) * 9.5367431640625e-7f;
                if (Time < 0.f || Time > 1.f)
                {
                    continue;
                }
                FVector Point;
                NativeSphereRayPoint(Center, Radius, Start, Direction, Point);
                const float DistanceSquared = FVector::DistSquared(Point, Start);
                if (DistanceSquared < ClosestDistanceSquared)
                {
                    ClosestDistanceSquared = DistanceSquared;
                    OutBone = Bone;
                    OutPoint = Point;
                }
            }
        }
    }
}
bRestrictZone = false;
return !OutBone.IsNone();
