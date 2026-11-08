// Original body: 0x1407c2090 (thunk 0x14080a550). Offset evidence: CALL 0x1407c5240 = UCommonButton::NativeGetButtonStyleDefault1407c5240(this) returns a UObject*. When it is non-null, 16 bytes at [RAX+0x470] are copied (MOVUPS) to the OutButtonPadding reference; a null result leaves OutButtonPadding unchanged.
// The returned object's class is not reflected, so the FMargin is read at its raw offset 0x470.
// CONFIDENCE: MEDIUM
const UObject* ResolvedStyle = UCommonButton::NativeGetButtonStyleDefault1407c5240(const_cast<UCommonButton*>(this));
if (ResolvedStyle != nullptr)
{
    OutButtonPadding = *reinterpret_cast<const FMargin*>(reinterpret_cast<const uint8*>(ResolvedStyle) + 0x470);
}
