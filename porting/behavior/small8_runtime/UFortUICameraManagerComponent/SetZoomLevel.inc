// Original body address: 0x141cb8880
// Offset evidence: Float at object offset 0x1ac. Own region (lowest own offset 0x0), not covered by any reflected member; recovered member NativeRecoveredFloat_0x1ac. Clamp constant 1.0f from runtime dump .rdata at 0x1459a86f8. Equal input returns early.
// CONFIDENCE: MEDIUM
if (ZoomLevel == NativeRecoveredFloat_0x1ac)
{
    return;
}
float NewZoom = 0.0f;
if (ZoomLevel >= 0.0f)
{
    NewZoom = (ZoomLevel < 1.0f) ? ZoomLevel : 1.0f;
}
NativeRecoveredFloat_0x1ac = NewZoom;
