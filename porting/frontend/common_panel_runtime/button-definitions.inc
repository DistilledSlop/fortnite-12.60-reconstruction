
#include "../Private/SCommonButton.h"
#include "Components/ButtonSlot.h"
#include "Widgets/Layout/SBox.h"
#include "UCommonUISubsystem.h"
#include "UCommonInputManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UCommonButtonInternal::UCommonButtonInternal(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bButtonEnabled = true;
    bInteractionEnabled = true;
}

void UCommonButtonInternal::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    NativeCommonButton.Reset();
    NativeButtonBox.Reset();
}

// Original complete factory: 0x1407e9150..0x1407e9bd1 (exclusive).
TSharedRef<SWidget> UCommonButtonInternal::RebuildWidget()
{
    // Each callback uses a weak UObject delegate instance in the original.
    NativeCommonButton = SNew(SCommonButton)
        .OnClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleCommonClicked))
        .OnDoubleClicked(BIND_UOBJECT_DELEGATE(FOnClicked, SlateHandleCommonDoubleClicked))
        .OnPressed(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleCommonPressed))
        .OnReleased(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleCommonReleased))
        .ButtonStyle(&WidgetStyle)
        .ClickMethod(ClickMethod)
        .TouchMethod(TouchMethod)
        .IsFocusable(IsFocusable)
        .ButtonEnabled(bButtonEnabled)
        .InteractionEnabled(bInteractionEnabled)
        .OnFocusReceived(BIND_UOBJECT_DELEGATE(FSimpleDelegate, SlateHandleCommonFocusReceived));

    // These must remain two distinct owning fields: original +478 and +418.
    MyButton = NativeCommonButton;

    NativeButtonBox = SNew(SBox)
        .MinDesiredWidth(static_cast<float>(MinWidth))
        .MinDesiredHeight(static_cast<float>(MinHeight))
        [
            NativeCommonButton.ToSharedRef()
        ];

    if (GetChildrenCount() > 0)
    {
        Cast<UButtonSlot>(GetContentSlot())->BuildSlot(NativeCommonButton.ToSharedRef());
    }

    return NativeButtonBox.ToSharedRef();
}

bool UCommonButtonInternal::IsCommonInputSuspended()
{
    UWorld* World = GetWorld();
    UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
    UCommonUISubsystem* Subsystem = GameInstance ? GameInstance->GetSubsystem<UCommonUISubsystem>() : nullptr;
    UCommonInputManager* Manager = Subsystem ? Subsystem->CommonInputManager : nullptr;
    return Manager && Manager->IsInputSuspended();
}

FReply UCommonButtonInternal::SlateHandleCommonClicked()
{
    if (IsCommonInputSuspended())
    {
        return FReply::Handled();
    }
    return UButton::SlateHandleClicked();
}

void UCommonButtonInternal::SlateHandleCommonPressed()
{
    if (!IsCommonInputSuspended())
    {
        UButton::SlateHandlePressed();
    }
}

void UCommonButtonInternal::SlateHandleCommonReleased()
{
    if (!IsCommonInputSuspended())
    {
        UButton::SlateHandleReleased();
    }
}

FReply UCommonButtonInternal::SlateHandleCommonDoubleClicked()
{
    if (IsCommonInputSuspended())
    {
        return FReply::Unhandled();
    }
    FReply Reply = FReply::Unhandled();
    if (NativeDoubleClickDelegate.IsBound())
    {
        Reply = NativeDoubleClickDelegate.Execute();
    }
    if (OnDoubleClicked.IsBound())
    {
        OnDoubleClicked.Broadcast();
        Reply = FReply::Handled();
    }
    return Reply;
}

void UCommonButtonInternal::SlateHandleCommonFocusReceived()
{
    NativeFocusReceivedDelegate.ExecuteIfBound();
}

void UCommonButtonInternal::NativeSetInteractionEnabled(bool bEnabled)
{
    if (bInteractionEnabled != bEnabled)
    {
        // Precondition: RebuildWidget must have created an SCommonButton.
        SCommonButton* const Button = NativeCommonButton.Get();
        bInteractionEnabled = bEnabled;
        if (Button != nullptr)
        {
            Button->SetInteractionEnabled(bEnabled);
        }
    }
}

// Original inline path in CommonButton::SetIsEnabled, 1407f78f0.
// This writes the internal and custom Slate flags without invoking UWidget's setter.
void UCommonButtonInternal::NativeSetButtonEnabled(bool bEnabled)
{
    bButtonEnabled = bEnabled;
    if (NativeCommonButton.IsValid())
    {
        NativeCommonButton->bButtonEnabled = bEnabled;
    }
}
