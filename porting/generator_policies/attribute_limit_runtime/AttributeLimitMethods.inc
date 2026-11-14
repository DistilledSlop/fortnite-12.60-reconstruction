// Inject inside FFortGameplayAttributeData; requires the existing policy's
// NativeOrderedDifferent and <xmmintrin.h>. Original 141bc4c50/141bc4cc0.
void NativeSetMaximum141bc4c50(float NewMaximum)
{
    if (!NativeOrderedDifferent(NewMaximum, Maximum)) return;
    Maximum = NewMaximum;
    // MAX setter's MINSS has NewMaximum as source: it wins unordered and ties.
    const auto Clamp = [this, NewMaximum](float Value) {
        if (!_mm_comige_ss(_mm_set_ss(Value), _mm_set_ss(Minimum))) return Minimum;
        return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(Value), _mm_set_ss(NewMaximum)));
    };
    CurrentValue = Clamp(UnclampedCurrentValue);
    bIsCurrentClamped = NativeOrderedDifferent(CurrentValue, UnclampedCurrentValue);
    if (bShouldClampBase)
    {
        BaseValue = Clamp(UnclampedBaseValue);
        bIsBaseClamped = NativeOrderedDifferent(BaseValue, UnclampedBaseValue);
    }
}

void NativeSetMinimum141bc4cc0(float NewMinimum)
{
    if (!NativeOrderedDifferent(NewMinimum, Minimum)) return;
    Minimum = NewMinimum;
    const auto Clamp = [this, NewMinimum](float Value) {
        if (!_mm_comige_ss(_mm_set_ss(Value), _mm_set_ss(NewMinimum))) return NewMinimum;
        return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(Maximum), _mm_set_ss(Value)));
    };
    CurrentValue = Clamp(UnclampedCurrentValue);
    bIsCurrentClamped = NativeOrderedDifferent(CurrentValue, UnclampedCurrentValue);
    if (bShouldClampBase)
    {
        BaseValue = Clamp(UnclampedBaseValue);
        bIsBaseClamped = NativeOrderedDifferent(BaseValue, UnclampedBaseValue);
    }
}
