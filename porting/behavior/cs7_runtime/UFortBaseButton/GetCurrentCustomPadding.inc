// Original address: 0x144d793e0 (thunk 0x145204740)
// Evidence: NativeFindButtonStyleEntry144d7bab0(this) gives the style entry (null returns a zero FMargin). BrushSize = byte [this+0x2a0] (reflected BrushSize). Margin = NativeGetMarginBySize141f894c0(entry+0x1998, BrushSize), returned by value.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortButtonStyle) > 0, "UFortButtonStyle must be complete for NativeGetMarginBySize141f894c0");
uint8* StyleEntry = UFortBaseButton::NativeFindButtonStyleEntry144d7bab0(const_cast<UFortBaseButton*>(this));
if (!StyleEntry)
{
	return FMargin(0.0f, 0.0f, 0.0f, 0.0f);
}
const FMargin* StyleMargin = UFortButtonStyle::NativeGetMarginBySize141f894c0(StyleEntry + 0x1998, static_cast<int32>(BrushSize.GetValue()));
return *StyleMargin;
