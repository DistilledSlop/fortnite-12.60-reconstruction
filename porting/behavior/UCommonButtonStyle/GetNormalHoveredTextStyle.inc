// CONFIDENCE: high; original wrapper and helper validate and return the existing default object.
// Original wrapper 0x14080b170 calls 0x1407c3f00; reflected class field is at 0x4a0.
UClass* TextStyleClass = NormalHoveredTextStyle.Get();
return TextStyleClass ? Cast<UCommonTextStyle>(TextStyleClass->GetDefaultObject(false)) : nullptr;
