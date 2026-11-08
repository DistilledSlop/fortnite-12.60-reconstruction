// Behaviour snippet: UFortStreamMediaSource::UpdatePlayerOptions (body only; spliced into the generated member function).
// Original address: 0x141f77c50 (thunk 0x142aeb790)
// Offset evidence: this+0x200 is NativeRecoveredFloat_0x200 (start time in seconds); this+0x1e0 is an unreflected byte in the own region (NativeRecoveredByte_0x1e0).
// Output: FMediaPlayerOptions.SeekTime (int64 ticks, struct offset 0x20) = trunc(start * 1e7f + 0.5f) when start > 0 and the byte is 0; the struct is returned unchanged otherwise.
// Verbosity-gated diagnostic log at 0x142c54b70 elided (no state change).
// CONFIDENCE: MEDIUM
if (NativeRecoveredFloat_0x200 > 0.0f && NativeRecoveredByte_0x1e0 == 0)
{
    const float ScaledStart = NativeRecoveredFloat_0x200 * 10000000.0f + 0.5f;
    PlayerOptions.SeekTime = FTimespan(static_cast<int64>(static_cast<double>(ScaledStart)));
}
return PlayerOptions;
