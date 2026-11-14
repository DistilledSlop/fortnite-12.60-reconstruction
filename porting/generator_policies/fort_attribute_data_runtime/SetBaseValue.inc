// 0x141bc47e0: changing limits alone never bypasses the input equality guard.
if (!NativeOrderedDifferent(NewValue, UnclampedBaseValue))
{
    return;
}
UnclampedBaseValue = NewValue;
const float ClampedValue = bShouldClampBase ? NativeClampValue(NewValue) : NewValue;
BaseValue = ClampedValue;
bIsBaseClamped = NativeOrderedDifferent(ClampedValue, NewValue);
