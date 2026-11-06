// CONFIDENCE: high
// Native target 0x1424bbc00 checks only the signed upper bound.
if (Index < CollisionSounds.Num())
{
    CollisionSounds.GetData()[Index].Asset = SoundToUse;
}
