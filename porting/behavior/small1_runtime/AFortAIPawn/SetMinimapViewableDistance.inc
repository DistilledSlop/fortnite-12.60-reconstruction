// Original address: 0x1422ec870 (AFortAIPawn::SetMinimapViewableDistance)
// Offset evidence: [RCX+0x1b30] is MinimapViewableDistance (reflected Float); [RCX+0x1d00] is MiniMapIndicator (reflected Object); float at +0x22c written raw on the pointee.
// CONFIDENCE: HIGH
MinimapViewableDistance = NewMinimapViewableDistance;
if (MiniMapIndicator != nullptr)
{
    *reinterpret_cast<float*>(reinterpret_cast<uint8*>(MiniMapIndicator) + 0x22c) = NewMinimapViewableDistance;
}
