// Behaviour snippet: UFortSwipePanel::GetSwipeInfo (body only; spliced into the generated member function).
// Original address: 0x144ebe9e0 (thunk 0x1452ffd10)
// Offset evidence: this+0x240 is NativeRecoveredPtr_0x240 (unreflected pointer in the own region); the pointee is read raw: +0x3f4 byte flag, +0x3e0 int32 index, +0x398/+0x39c/+0x3ec/+0x3f0 floats, +0x5c/+0x48 at base +0x398.
// Null pointer: nothing is written. Otherwise OutIndex = -1 when the flag byte is set, else the int32 at +0x3e0.
// Percent defaults to (0, 0) from writable global snapshots 0x148218628/0x14821862c (value 0.0, used as zero fallbacks); when flag byte is 0 and index != -1, X = +0x3ec / +0x398 and Y = +0x3f0 / +0x39c for positive denominators.
// CONFIDENCE: MEDIUM
const uint8* SwipeSource = reinterpret_cast<const uint8*>(NativeRecoveredPtr_0x240);
if (SwipeSource == nullptr)
{
    return;
}
OutIndex = (*reinterpret_cast<const uint8*>(SwipeSource + 0x3f4) == 0)
    ? *reinterpret_cast<const int32*>(SwipeSource + 0x3e0)
    : -1;
const uint8* Sub = SwipeSource + 0x398;
float PctX = 0.0f;
float PctY = 0.0f;
if (*reinterpret_cast<const uint8*>(Sub + 0x5c) == 0 && *reinterpret_cast<const int32*>(Sub + 0x48) != -1)
{
    const float DenomX = *reinterpret_cast<const float*>(Sub + 0x0);
    if (DenomX > 0.0f)
    {
        PctX = *reinterpret_cast<const float*>(Sub + 0x54) / DenomX;
    }
    const float DenomY = *reinterpret_cast<const float*>(Sub + 0x4);
    if (DenomY > 0.0f)
    {
        PctY = *reinterpret_cast<const float*>(Sub + 0x58) / DenomY;
    }
}
OutSwipePercentage.X = PctX;
OutSwipePercentage.Y = PctY;
