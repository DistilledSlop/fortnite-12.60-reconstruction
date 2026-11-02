#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "ECommonInputType.h"
#include "Framework/Application/AnalogCursor.h"
#include "Rendering/SlateRenderTransform.h"

class UCommonUIActionRouter;
class SWidget;
class UWidget;

class FCommonAnalogCursor : public FAnalogCursor
{
public:
    explicit FCommonAnalogCursor(UCommonUIActionRouter& InRouter);
    void Initialize();
    virtual void Tick(float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
    virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& Event) override;
    virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& Event) override;
    virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& Event) override;
    virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& Event) override;
    virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& Event) override;
    virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& Event) override;
    virtual int32 GetOwnerUserIndex() const override;

protected:
    virtual bool IsRelevantInput(const FAnalogInputEvent& Event) const override;
    virtual bool IsRelevantInput(const FKeyEvent& Event) const override;
    bool IsRelevantKeyInput(const FKeyEvent& Event) const;

private:
    bool NativeViewportBlocksKeyDown() const;
    void HandleInputMethodChanged(ECommonInputType InputType);
    void NativeUpdateCursorVisibility();
    bool NativeShouldHandleInput() const;
    void NativeSetCursorPositionInViewport(const FVector2D& NormalizedPosition);
    virtual EOrientation NativeGetScrollOrientation(const UWidget* Widget) const;
    UCommonUIActionRouter& Router;
    TSet<FKey> PressedMouseKeys;
    TWeakPtr<SWidget> LastCursorTarget;
    FSlateRenderTransform NativeLastCursorTargetTransform;
    float NativeTimeUntilScrollUpdate = 0.0f;
    ECommonInputType CurrentInputMethod = ECommonInputType::MouseAndKeyboard;
    bool bIsGamepadCursor = false;
    bool bUnknownD6 = true;
};

TSharedRef<FCommonAnalogCursor> NativeCreateCommonAnalogCursor(UCommonUIActionRouter& Router);
