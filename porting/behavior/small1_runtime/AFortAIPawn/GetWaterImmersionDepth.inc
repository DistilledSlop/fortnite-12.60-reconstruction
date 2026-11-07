// Original address: 0x1422d9ad0 (AFortAIPawn::GetWaterImmersionDepth)
// Offset evidence: [RCX+0x1e88] is CurrentWaterBody (reflected Object); [RCX+0x280] is CharacterMovement (reflected Object);
// float at CharacterMovement+0x1d08 read raw (pointee type not reflected). Null path returns 0xbf800000 (-1.0f) per decompile.
// CONFIDENCE: HIGH
if (CurrentWaterBody == nullptr)
{
    return -1.0f;
}
const uint8* MovementRaw = reinterpret_cast<const uint8*>(CharacterMovement);
return *reinterpret_cast<const float*>(MovementRaw + 0x1d08);
