// Original address: 0x14287d860.
// CONFIDENCE: HIGH
// Exact CVTTSS2SI conversion, including its invalid-conversion result.
const double Value = static_cast<double>(ColorBlindStrength);
if (!(Value >= -2147483648.0 && Value < 2147483648.0))
{
    return (-2147483647 - 1);
}
return static_cast<int32>(Value);
