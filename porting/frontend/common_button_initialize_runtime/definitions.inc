#include "Blueprint/WidgetTree.h"
#include "Components/ButtonSlot.h"
#include "UCommonButtonInternal.h"

bool UCommonButton::Initialize()
{
    // Parent can synchronously call NativeOnInitialized before the internal exists.
    if (!Super::Initialize()) return false;

    UCommonButtonInternal* Internal = WidgetTree->ConstructWidget<UCommonButtonInternal>(
        UCommonButtonInternal::StaticClass(), TEXT("InternalRootButton"));
    Internal->ClickMethod = ClickMethod;
    Internal->TouchMethod = TouchMethod;
    Internal->PressMethod = PressMethod;
    Internal->IsFocusable = bIsFocusable;
    Internal->NativeSetButtonEnabled(bNativeButtonEnabled);
    Internal->NativeSetInteractionEnabled(bNativeInteractionEnabled);
    NativeInternalButton = Internal;

    if (UWidget* OldRoot = WidgetTree->RootWidget)
    {
        UButtonSlot* Slot = Cast<UButtonSlot>(Internal->AddChild(OldRoot));
        Slot->SetPadding(FMargin(0.0f));
        Slot->SetHorizontalAlignment(HAlign_Fill);
        Slot->SetVerticalAlignment(VAlign_Fill);
        WidgetTree->RootWidget = Internal;

        Internal->OnClicked.AddUniqueDynamic(this, &UCommonButton::HandleButtonClicked);
        Internal->NativeDoubleClickDelegate.BindUObject(this, &UCommonButton::NativeHandleDoubleClicked);
        Internal->NativeFocusReceivedDelegate.BindUObject(this, &UCommonButton::NativeHandleFocusReceived);
        Internal->OnPressed.AddUniqueDynamic(this, &UCommonButton::HandleButtonPressed);
        Internal->OnReleased.AddUniqueDynamic(this, &UCommonButton::HandleButtonReleased);
    }
    return true;
}
