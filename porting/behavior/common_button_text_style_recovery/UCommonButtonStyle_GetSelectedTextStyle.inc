// CONFIDENCE: high; original wrapper and helper validate and return the existing default object.
// Original wrapper 0x14080b830 calls 0x1407c4df0; reflected class field is at 0x4a8.
UClass* TextStyleClass = SelectedTextStyle.Get();
return TextStyleClass ? Cast<UCommonTextStyle>(TextStyleClass->GetDefaultObject(false)) : nullptr;
