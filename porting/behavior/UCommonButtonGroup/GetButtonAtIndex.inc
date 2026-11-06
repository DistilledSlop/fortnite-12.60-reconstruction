// CONFIDENCE: HIGH
// Reviewed against captured wrapper and implementation; build and runtime pending.
if (!NativeButtonGroupWidgets.IsValidIndex(Index))
{
    return nullptr;
}
return NativeButtonGroupWidgets[Index].Get();
