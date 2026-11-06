// CONFIDENCE: high; original wrapper and helper validate and return the existing default object.
// Original wrapper 0x14080b680 calls 0x1407c4920; reflected class field is at 0x4b0.
UClass* TextStyleClass = SelectedHoveredTextStyle.Get();
return TextStyleClass ? Cast<UCommonTextStyle>(TextStyleClass->GetDefaultObject(false)) : nullptr;
