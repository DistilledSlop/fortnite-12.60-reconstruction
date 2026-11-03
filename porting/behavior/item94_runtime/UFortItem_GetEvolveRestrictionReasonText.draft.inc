// CONFIDENCE: LOW -- DRAFT, UNCOUNTED. Keys and source text remain unresolved.
// Native 0x14202d350 dispatches on the raw enum byte and constructs an FText
// with namespace "FortItem"; the returned FText owns its reference normally.
switch (static_cast<uint8>(Reason))
{
case 0: return NativeLocalizedText_142bedc40(Key_14606e6a8, TEXT("FortItem"), Source_14606e650);
case 1: return NativeLocalizedText_142bedc40(Key_14606e760, TEXT("FortItem"), Source_14606e6f0);
case 2: return NativeLocalizedText_142bedc40(Key_14606e830, TEXT("FortItem"), Source_14606e7d0);
case 3: return NativeLocalizedText_142bedc40(Key_14606e910, TEXT("FortItem"), Source_14606e8b0);
case 4: return NativeLocalizedText_142bedc40(Key_14606e9a0, TEXT("FortItem"), Source_14606e940);
case 5: return NativeLocalizedText_142bedc40(Key_14606ea50, TEXT("FortItem"), Source_14606e9f0);
case 6: return NativeLocalizedText_142bedc40(Key_14606eb00, TEXT("FortItem"), Source_14606eaa0);
default: return NativeDefaultText_1407c2c80();
}
