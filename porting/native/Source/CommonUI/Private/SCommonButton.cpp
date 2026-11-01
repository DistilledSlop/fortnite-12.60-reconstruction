#include "SCommonButton.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"

SCommonButton::SCommonButton()
    : bButtonEnabled(false), bInteractionEnabled(false)
{
}

SCommonButton::~SCommonButton() = default;

bool SCommonButton::IsHovered() const
{
    return bInteractionEnabled && SButton::IsHovered();
}

void SCommonButton::SetInteractionEnabled(bool bEnabled)
{
    if (bInteractionEnabled != bEnabled)
    {
        const bool bWasHovered = IsHovered();
        bInteractionEnabled = bEnabled;
        const bool bNowHovered = IsHovered();
        if (bWasHovered != bNowHovered)
        {
            if (bNowHovered) OnHovered.ExecuteIfBound();
            else OnUnhovered.ExecuteIfBound();
        }
    }
}


void SCommonButton::Construct(const FArguments& InArgs)
{
    OnDoubleClicked = InArgs._OnDoubleClicked;

    // A fresh base argument object is deliberate: the original forwards exactly
    // these fields; SButton owns all other attributes, delegates, text and sounds.
    SButton::Construct(SButton::FArguments()
        .ButtonStyle(InArgs._ButtonStyle)
        .HAlign(InArgs._HAlign)
        .VAlign(InArgs._VAlign)
        .ClickMethod(InArgs._ClickMethod)
        .TouchMethod(InArgs._TouchMethod)
        .PressMethod(InArgs._PressMethod)
        .OnClicked(InArgs._OnClicked)
        .OnPressed(InArgs._OnPressed)
        .OnReleased(InArgs._OnReleased)
        .IsFocusable(InArgs._IsFocusable)
        [ InArgs._Content.Widget ]);

    SetCanTick(false);
    FocusReceivedDelegate = InArgs._OnFocusReceived;
    bButtonEnabled = InArgs._ButtonEnabled;
    bInteractionEnabled = InArgs._InteractionEnabled;
    CacheVolatility();
}

FReply SCommonButton::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
    FReply Reply = SButton::OnFocusReceived(MyGeometry, InFocusEvent);
    FocusReceivedDelegate.ExecuteIfBound();
    return Reply;
}

FReply SCommonButton::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Enter)
    {
        return FReply::Unhandled();
    }
    return SButton::OnKeyDown(MyGeometry, InKeyEvent);
}

FReply SCommonButton::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Enter)
    {
        return FReply::Unhandled();
    }
    return SButton::OnKeyUp(MyGeometry, InKeyEvent);
}

FReply SCommonButton::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (IsInteractable())
    {
        return SButton::OnMouseButtonDown(MyGeometry, MouseEvent);
    }
    return FReply::Handled();
}

FReply SCommonButton::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    FReply Reply = FReply::Handled();
    if (IsInteractable())
    {
        Reply = SButton::OnMouseButtonUp(MyGeometry, MouseEvent);
    }
    else if (HasMouseCapture())
    {
        Release();
        Reply.ReleaseMouseCapture();
    }
    return Reply;
}

void SCommonButton::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (!MouseEvent.IsTouchEvent())
    {
        SButton::OnMouseEnter(MyGeometry, MouseEvent);
    }
}

void SCommonButton::OnMouseLeave(const FPointerEvent& MouseEvent)
{
    if (MouseEvent.IsTouchEvent())
    {
        if (HasMouseCapture())
        {
            Release();
        }
    }
    else
    {
        SButton::OnMouseLeave(MouseEvent);
    }
}

FReply SCommonButton::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (!IsInteractable())
    {
        return FReply::Handled();
    }
    if (OnDoubleClicked.IsBound())
    {
        FReply Reply = OnDoubleClicked.Execute();
        if (Reply.IsEventHandled())
        {
            return Reply;
        }
    }
    return SButton::OnMouseButtonDoubleClick(MyGeometry, MouseEvent);
}

FReply SCommonButton::OnTouchMoved(const FGeometry& MyGeometry, const FPointerEvent& TouchEvent)
{
    FReply Reply = FReply::Handled();
    if (HasMouseCapture())
    {
        if (!MyGeometry.IsUnderLocation(TouchEvent.GetScreenSpacePosition()))
        {
            Release();
            Reply.ReleaseMouseCapture();
        }
    }
    else
    {
        Reply = SButton::OnTouchMoved(MyGeometry, TouchEvent);
    }
    return Reply;
}

int32 SCommonButton::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
    const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
    int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    return SButton::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements,
        LayerId, InWidgetStyle, bParentEnabled && bButtonEnabled);
}

bool SCommonButton::IsInteractable() const
{
    return bButtonEnabled && bInteractionEnabled;
}

bool SCommonButton::IsPressed() const
{
    return IsInteractable() && SButton::IsPressed();
}
