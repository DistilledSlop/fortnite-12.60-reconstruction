// CONFIDENCE: MEDIUM. Original addresses: exec 0x142a7ccb0; helper calls FScalableFloat_GetValueAtLevel (named engine helper).
// Field names at +0x260 (float: zipline exit time, -1 means never) and +0x269 (FScalableFloat: jump-exit buffer) are NativeRecovered; world time is read at UWorld+0x540 (TimeSeconds, MEDIUM).
const float ZiplineExitTime = NativeRecoveredFloat_0x260;
if (FMath::Abs(ZiplineExitTime - (-1.0f)) > 1e-8f)
{
    const float ExitBuffer = NativeRecoveredScalableFloat_0x269.GetValueAtLevel(0.0f);
    const float WorldTimeSeconds = GetWorld()->TimeSeconds;
    if (WorldTimeSeconds - ZiplineExitTime < ExitBuffer)
    {
        return true;
    }
}
return false;
