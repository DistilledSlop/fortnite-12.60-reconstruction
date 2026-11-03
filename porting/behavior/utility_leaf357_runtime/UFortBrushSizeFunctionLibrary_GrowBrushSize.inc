// CONFIDENCE: high; native capture and reflection contracts reviewed.
{
    const uint32 Raw = static_cast<uint32>(OriginalBrushSize) + static_cast<uint32>(Steps);
    const int32 Signed = Raw <= 0x7fffffffu ? static_cast<int32>(Raw) : static_cast<int32>(static_cast<int64>(Raw) - 0x100000000ll);
    return static_cast<EFortBrushSize::Type>(FMath::Clamp(Signed, 0, 5));
}
