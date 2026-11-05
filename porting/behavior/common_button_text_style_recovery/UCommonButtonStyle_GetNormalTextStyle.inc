// CONFIDENCE: high; original wrapper and helper validate and return the existing default object.
// Original wrapper 0x14080b320 calls 0x1407c3fe0; reflected class field is at 0x498.
UClass* TextStyleClass = NormalTextStyle.Get();
return TextStyleClass ? Cast<UCommonTextStyle>(TextStyleClass->GetDefaultObject(false)) : nullptr;
