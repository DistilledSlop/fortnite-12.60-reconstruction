#include "CommonButtonTextStyleCore.h"
#include "UCommonButtonStyle.h"
#include "UCommonTextStyle.h"
#include "UCommonButtonInternal.h"

bool UCommonButton::IsHovered() const
{
    return NativeInternalButton.IsValid() && NativeInternalButton.Get()->IsHovered();
}

bool UCommonButton::GetSelected() const
{
    return bNativeSelected;
}

TSubclassOf<UCommonTextStyle> UCommonButton::GetCurrentTextStyleClass() const
{
    using CommonButtonTextStyleCore::Candidate;
    return CommonButtonTextStyleCore::Get(
        [this]() -> UCommonButtonStyle* {
            UClass* Class = Style.Get();
            if (!Class || !Class->IsChildOf(UCommonButtonStyle::StaticClass())) return nullptr;
            return Cast<UCommonButtonStyle>(Class->GetDefaultObject(false));
        },
        [this]() { return GetIsEnabled(); },
        [this]() { return GetSelected(); },
        [this]() { return IsHovered(); },
        [](UCommonButtonStyle* Owner, Candidate Which) -> UCommonTextStyle* {
            UClass* Class = nullptr;
            switch (Which) {
                case Candidate::Normal: Class = Owner->NormalTextStyle.Get(); break;
                case Candidate::NormalHovered: Class = Owner->NormalHoveredTextStyle.Get(); break;
                case Candidate::Selected: Class = Owner->SelectedTextStyle.Get(); break;
                case Candidate::SelectedHovered: Class = Owner->SelectedHoveredTextStyle.Get(); break;
                case Candidate::Disabled: Class = Owner->DisabledTextStyle.Get(); break;
            }
            if (!Class || !Class->IsChildOf(UCommonTextStyle::StaticClass())) return nullptr;
            return Cast<UCommonTextStyle>(Class->GetDefaultObject(false));
        },
        [](UCommonTextStyle* Object) -> TSubclassOf<UCommonTextStyle> {
            return Object->GetClass();
        });
}
