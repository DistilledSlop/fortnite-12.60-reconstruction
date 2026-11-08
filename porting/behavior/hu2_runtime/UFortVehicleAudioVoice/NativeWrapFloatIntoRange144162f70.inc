// Helper 0x144162f70 (static on UFortVehicleAudioVoice).
// Evidence: porting/generated/native-cap-helpers4/0x144162f70.asm, 52 instructions, no calls.
// ABI: XMM0 = Lo, XMM1 = Hi, R8 = float* Value (read and written), R9 = int32* WrapCount.
// Constant [0x1459a86f0] = 0.5f, read from the PE .rdata section.
// Caller evidence: 0x1441655c0 loads XMM0 = lower bound, XMM1 = upper bound, R8 = &value, R9 = &count.
// Behavior: wraps *Value into [Lo, Hi] by whole periods of (Hi - Lo); *WrapCount receives the period count (absolute value at the end).
// CONFIDENCE: MEDIUM
const float RangeWidth = Hi - Lo;
const float OriginalValue = *Value;
float NewValue = OriginalValue;

// Period count from a ratio: round(2r - 0.5) with round-half-to-even (CVTSS2SI), then arithmetic shift right by one.
auto WrapPeriodCount = [](float Ratio) -> int32
{
    const float ScaledY = (Ratio + Ratio) - 0.5f;
    const float FloorY = FMath::FloorToFloat(ScaledY);
    const float FracY = ScaledY - FloorY;
    int32 RoundedY = static_cast<int32>(FloorY);
    if (FracY > 0.5f || (FracY == 0.5f && (RoundedY & 1) != 0))
    {
        RoundedY += 1;
    }
    return RoundedY >> 1;
};

if (OriginalValue > Hi)
{
    const int32 Periods = WrapPeriodCount((Hi - OriginalValue) / RangeWidth);
    *WrapCount = Periods;
    NewValue = static_cast<float>(Periods) * RangeWidth + OriginalValue;
    *Value = NewValue;
}
else if (OriginalValue < Lo)
{
    const int32 Periods = WrapPeriodCount((OriginalValue - Lo) / RangeWidth);
    *WrapCount = Periods;
    NewValue = OriginalValue - static_cast<float>(Periods) * RangeWidth;
    *Value = NewValue;
}

// Boundary clamp for results that land exactly on an endpoint (asm 0x144163002..0x144163022).
bool bCheckUpperTail = false;
if (NewValue == Hi && OriginalValue < Lo)
{
    *Value = Lo;
    bCheckUpperTail = true;
}
else if (NewValue == Lo)
{
    bCheckUpperTail = true;
}
if (bCheckUpperTail && OriginalValue > Hi)
{
    *Value = Hi;
}

// Absolute value of the stored count, same CDQ/XOR/SUB sequence as the asm.
const int32 CountBeforeAbs = *WrapCount;
const int32 CountSign = CountBeforeAbs >> 31;
*WrapCount = (CountBeforeAbs ^ CountSign) - CountSign;
