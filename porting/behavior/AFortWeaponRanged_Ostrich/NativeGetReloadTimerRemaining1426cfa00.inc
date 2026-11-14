// CONFIDENCE: HIGH. Original 0x1426cfa00.
if (!GetWorld())
{
    return 0.0f;
}
return GetWorld()->GetTimerManager().GetTimerRemaining(NativeReloadTimer1108);
