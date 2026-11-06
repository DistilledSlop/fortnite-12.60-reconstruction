// CONFIDENCE: high; original wrapper and helper validate and return the existing default object.
// Original wrapper 0x14080a920 calls 0x1407c26f0; reflected class field is at 0x4b8.
UClass* TextStyleClass = DisabledTextStyle.Get();
return TextStyleClass ? Cast<UCommonTextStyle>(TextStyleClass->GetDefaultObject(false)) : nullptr;
