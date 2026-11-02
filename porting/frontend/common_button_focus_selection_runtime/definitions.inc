#include "UCommonActionWidget.h"
#include "UCommonButtonInternal.h"
#include "UCommonInputSubsystem.h"
#include "UCommonInputSettings.h"
#include "UCommonUIInputData.h"
#include "CommonInputModuleRuntime.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"

void UCommonButton::NativeRefreshActionWidgetVisibility()
{
    if (!InputActionWidget) return;
    ULocalPlayer* Player = GetOwningLocalPlayer();
    UCommonInputSubsystem* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
    bool bHide;
    if (bHideInputAction || (Input && bHideInputActionWithKeyboard &&
        Input->GetCurrentInputType() != ECommonInputType::Gamepad))
        bHide = true;
    else if (bNativeSelected)
        bHide = !bInteractableWhenSelected && !bToggleable && !bDisplayInputActionWhenNotInteractable;
    else
        bHide = !bDisplayInputActionWhenNotInteractable && !bNativeInteractionEnabled;
    // Input resolution may reenter; the original reloads the widget without a second null check.
    InputActionWidget->NativeHidden = bHide;
    InputActionWidget->NativeRefreshInputAction();
}

void UCommonButton::NativeUpdateActionWidget()
{
    if (!InputActionWidget) return;
    FDataTableRowHandle Row;
    if (!TriggeringInputAction.IsNull()) Row = TriggeringInputAction;
    else if (!NativeTriggeredInputAction.IsNull()) Row = NativeTriggeredInputAction;
    else if (bShouldUseFallbackDefaultInputAction && IsHovered())
    {
        UCommonInputSettings* Settings = GetCommonInputSettings();
        UClass* InputDataClass = Settings->InputDataClass.Get();
        if (InputDataClass && InputDataClass->IsChildOf(UCommonUIInputData::StaticClass()))
        {
            UObject* Default = InputDataClass->GetDefaultObject(false);
            if (!Default)
            {
                InputDataClass->GetDefaultObject();
                Default = InputDataClass->GetDefaultObject(false);
            }
            if (Default) Row = static_cast<UCommonUIInputData*>(Default)->DefaultClickAction;
        }
    }
    InputActionWidget->SetInputAction(Row);
    NativeRefreshActionWidgetVisibility();
}

void UCommonButton::NativeOnHovered()
{
    BP_OnHovered();
    NativeHovered.Broadcast();
    if (OnButtonHovered.IsBound()) OnButtonHovered.Broadcast(this);
    Invalidate(EInvalidateWidget::Layout);
    NativeOnCurrentTextStyleChanged();
    NativeUpdateActionWidget();
}

void UCommonButton::NativeOnUnhovered()
{
    BP_OnUnhovered();
    NativeUnhovered.Broadcast();
    if (OnButtonUnhovered.IsBound()) OnButtonUnhovered.Broadcast(this);
    Invalidate(EInvalidateWidget::Layout);
    NativeOnCurrentTextStyleChanged();
    NativeUpdateActionWidget();
}

void UCommonButton::NativeSetSelectedInternal(bool bSelected, bool bGiveClickFeedback, bool bBroadcast)
{
    bNativeSelected = bSelected;
    NativeRefreshCurrentStyle();
    if (bNativeSelected)
    {
        NativeOnSelected(bBroadcast);
        if (!bToggleable)
            NativeInternalButton.Get()->NativeSetInteractionEnabled(bInteractableWhenSelected);
        if (bGiveClickFeedback)
            FSlateApplication::Get().PlaySound(NormalStyle.PressedSlateSound, 0);
    }
    else
    {
        NativeInternalButton.Get()->NativeSetInteractionEnabled(bNativeInteractionEnabled);
        NativeOnDeselected(bBroadcast);
    }
    NativeRefreshActionWidgetVisibility();
}

void UCommonButton::NativeSetIsSelected(bool bSelected, bool bGiveClickFeedback)
{
    const bool bWasHovered = IsHovered();
    if (bSelectable && bSelected != bNativeSelected)
    {
        if (bSelected) NativeSetSelectedInternal(true, bGiveClickFeedback, true);
        else if (bToggleable) NativeSetSelectedInternal(false, true, true);
    }
    const bool bNowHovered = IsHovered();
    if (bWasHovered != bNowHovered)
    {
        if (bNowHovered) NativeOnHovered(); else NativeOnUnhovered();
    }
}

void UCommonButton::NativeHandleFocusReceived()
{
    // Original predicate 1406801e0 is xor al,al; ret in this build.
    if (bShouldSelectUponReceivingFocus)
    {
        const bool bWasHovered = IsHovered();
        if (bSelectable && !bNativeSelected) NativeSetSelectedInternal(true, false, true);
        const bool bNowHovered = IsHovered();
        if (bWasHovered != bNowHovered)
        {
            if (bNowHovered) NativeOnHovered(); else NativeOnUnhovered();
        }
    }
    NativeFocusReceived.Broadcast();
}

void UCommonButton::HandleButtonPressed()
{
    ULocalPlayer* Player = GetOwningLocalPlayer();
    UCommonInputSubsystem* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
    if (Input && Input->GetCurrentInputType() == ECommonInputType::Touch) NativeOnHovered();
}

void UCommonButton::HandleButtonReleased()
{
    ULocalPlayer* Player = GetOwningLocalPlayer();
    UCommonInputSubsystem* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
    if (Input && Input->GetCurrentInputType() == ECommonInputType::Touch) NativeOnUnhovered();
}

void UCommonButton::HandleButtonClicked()
{
    const ESlateVisibility Visibility = GetVisibility();
    if (GetIsEnabled() && bNativeButtonEnabled && bNativeInteractionEnabled &&
        Visibility != ESlateVisibility::Collapsed && Visibility != ESlateVisibility::Hidden)
    {
        NativeSetIsSelected(!bNativeSelected, false);
        NativeOnClicked();
        NativeDispatchTriggeredAction();
    }
}
