#pragma once
#include "Widgets/Input/SButton.h"

// Reconstructed Slate behavior. Original UObject factory wiring is separate.
class SCommonButton : public SButton
{
public:
    SLATE_BEGIN_ARGS(SCommonButton)
        : _Content()
        , _HAlign(HAlign_Fill)
        , _VAlign(VAlign_Fill)
        , _ClickMethod(EButtonClickMethod::DownAndUp)
        , _TouchMethod(EButtonTouchMethod::DownAndUp)
        , _PressMethod(EButtonPressMethod::DownAndUp)
        , _IsFocusable(true)
        , _InteractionEnabled(true)
        {}
        SLATE_DEFAULT_SLOT(FArguments, Content)                  // +1a0
        // Original args constructor 140792780 leaves +1b0 and +220 unwritten.
        // EVERY caller must explicitly supply ButtonStyle and ButtonEnabled.
        // Do not add guessed fallback style or guessed enabled default here.
        SLATE_ARGUMENT(const FButtonStyle*, ButtonStyle)         // +1b0
        SLATE_ARGUMENT(EHorizontalAlignment, HAlign)             // +1b8
        SLATE_ARGUMENT(EVerticalAlignment, VAlign)               // +1bc
        SLATE_EVENT(FOnClicked, OnClicked)                       // +1c0
        SLATE_EVENT(FOnClicked, OnDoubleClicked)                 // +1d0
        SLATE_EVENT(FSimpleDelegate, OnPressed)                  // +1e0
        SLATE_EVENT(FSimpleDelegate, OnReleased)                 // +1f0
        SLATE_ARGUMENT(EButtonClickMethod::Type, ClickMethod)    // +200
        SLATE_ARGUMENT(EButtonTouchMethod::Type, TouchMethod)    // +204
        SLATE_ARGUMENT(EButtonPressMethod::Type, PressMethod)    // +208
        SLATE_ARGUMENT(bool, IsFocusable)                       // +20c
        SLATE_EVENT(FSimpleDelegate, OnFocusReceived)             // +210
        SLATE_ARGUMENT(bool, ButtonEnabled)                     // +220
        SLATE_ARGUMENT(bool, InteractionEnabled)                // +221
    SLATE_END_ARGS()

    SCommonButton();
    virtual ~SCommonButton() override;
    void Construct(const FArguments& InArgs);
    virtual FReply OnFocusReceived(const FGeometry&, const FFocusEvent&) override;
    virtual FReply OnKeyDown(const FGeometry&, const FKeyEvent&) override;
    virtual FReply OnKeyUp(const FGeometry&, const FKeyEvent&) override;
    virtual FReply OnMouseButtonDown(const FGeometry&, const FPointerEvent&) override;
    virtual FReply OnMouseButtonUp(const FGeometry&, const FPointerEvent&) override;
    virtual FReply OnMouseButtonDoubleClick(const FGeometry&, const FPointerEvent&) override;
    virtual void OnMouseEnter(const FGeometry&, const FPointerEvent&) override;
    virtual void OnMouseLeave(const FPointerEvent&) override;
    virtual FReply OnTouchMoved(const FGeometry&, const FPointerEvent&) override;
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry&, const FSlateRect&,
        FSlateWindowElementList&, int32, const FWidgetStyle&, bool) const override;
    virtual bool IsInteractable() const override;
    virtual bool IsHovered() const override;
    virtual bool IsPressed() const override;
    void SetInteractionEnabled(bool bEnabled);
private:
    friend class UCommonButtonInternal;
    FOnClicked OnDoubleClicked;
    FSimpleDelegate FocusReceivedDelegate;
    bool bButtonEnabled;
    bool bInteractionEnabled;
};
