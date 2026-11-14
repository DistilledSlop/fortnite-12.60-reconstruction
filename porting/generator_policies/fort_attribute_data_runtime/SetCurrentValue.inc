// 0x141bc4960: UCOMISS/JE returns for equality OR unordered operands.
if (!NativeOrderedDifferent(NewValue, UnclampedCurrentValue))
{
    return;
}
UnclampedCurrentValue = NewValue;
const float ClampedValue = NativeClampValue(NewValue);
CurrentValue = ClampedValue;
bIsCurrentClamped = NativeOrderedDifferent(ClampedValue, NewValue);
