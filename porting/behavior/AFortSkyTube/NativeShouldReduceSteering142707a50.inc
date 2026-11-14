// Original address: 0x142707a50
// CONFIDENCE: high
if (FMath::Abs(InputVector.X) > 0.0001f || FMath::Abs(InputVector.Y) > 0.0001f || FMath::Abs(InputVector.Z) > 0.0001f)
{
    if (FVector::DotProduct(AlignedAxis, InputVector) >= 0.0001f ||
        FVector::DotProduct(AlignedAxis, ReferenceDir) <= 0.0f)
    {
        return false;
    }
}
return true;
