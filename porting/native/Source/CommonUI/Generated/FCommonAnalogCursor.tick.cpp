#include "FCommonAnalogCursor.h"
#include "UCommonUIActionRouter.h"
#include "UCommonUIInputSettings.h"
#include "CommonActionBindingRoute.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "Components/ListView.h"
#include "Components/ScrollBar.h"
#include "Components/ScrollBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Framework/Views/TableViewMetadata.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/SViewport.h"
#include "Slate/SGameLayerManager.h"

// Shipping compares the four matrix elements with 1e-4 tolerance and the
// translation exactly. FTransform2D::Equals also tolerates translation, so do
// the comparison explicitly through the public matrix/vector interface.
static bool CommonCursorTransformEquals(const FSlateRenderTransform& A, const FSlateRenderTransform& B)
{
    const FMatrix2x2 MA = A.GetMatrix();
    const FMatrix2x2 MB = B.GetMatrix();
    float A0, A1, A2, A3, B0, B1, B2, B3;
    MA.GetMatrix(A0, A1, A2, A3);
    MB.GetMatrix(B0, B1, B2, B3);
    return FMath::Abs(A0 - B0) <= 0.0001f && FMath::Abs(A1 - B1) <= 0.0001f
        && FMath::Abs(A2 - B2) <= 0.0001f && FMath::Abs(A3 - B3) <= 0.0001f
        && A.GetTranslation() == B.GetTranslation();
}

void FCommonAnalogCursor::NativeUpdateCursorVisibility()
{
    if (TSharedPtr<FSlateUser> User = FSlateApplication::Get().GetUser(GetOwnerUserIndex()))
    {
        ULocalPlayer* Player = Router.GetLocalPlayer();
        User->SetCursorVisibility(bIsGamepadCursor || RouteShouldShowCursorForTouch(Player->ViewportClient)
            || CurrentInputMethod == ECommonInputType::MouseAndKeyboard);
    }
}

bool FCommonAnalogCursor::NativeShouldHandleInput() const
{
    ULocalPlayer* Player = Router.GetLocalPlayer();
    UGameViewportClient* ViewportClient = Player ? Player->ViewportClient : nullptr;
    TSharedPtr<SViewport> Viewport = ViewportClient ? ViewportClient->GetGameViewportWidget() : nullptr;
    if (Viewport.IsValid())
        if (TSharedPtr<FSlateUser> User = FSlateApplication::Get().GetUser(GetOwnerUserIndex()))
            return !User->DoesWidgetHaveCursorCapture(Viewport) && User->IsWidgetInFocusPath(Viewport);
    return false;
}

bool FCommonAnalogCursor::IsRelevantInput(const FAnalogInputEvent& Event) const
{
    return CurrentInputMethod == ECommonInputType::Gamepad && FAnalogCursor::IsRelevantInput(Event) && NativeShouldHandleInput();
}

bool FCommonAnalogCursor::IsRelevantInput(const FKeyEvent& Event) const
{
    return IsRelevantKeyInput(Event);
}

bool FCommonAnalogCursor::IsRelevantKeyInput(const FKeyEvent& Event) const
{
    if (CurrentInputMethod != ECommonInputType::Gamepad || !FAnalogCursor::IsRelevantInput(static_cast<const FInputEvent&>(Event)))
        return false;
    if (NativeShouldHandleInput())
        return true;
    if (Event.GetKey() != EKeys::Virtual_Accept || !Router.NativeActionInputConfig.IsSet())
        return false;
    const uint8 CaptureMode = static_cast<uint8>(Router.NativeActionInputConfig->MouseCaptureMode);
    return CaptureMode >= 2 && CaptureMode <= 3;
}

void FCommonAnalogCursor::NativeSetCursorPositionInViewport(const FVector2D& NormalizedPosition)
{
    TSharedPtr<FSlateUser> User = FSlateApplication::Get().GetUser(GetOwnerUserIndex());
    ULocalPlayer* Player = Router.GetLocalPlayer();
    TSharedPtr<SViewport> Viewport = Player && Player->ViewportClient ? Player->ViewportClient->GetGameViewportWidget() : nullptr;
    if (User.IsValid() && Viewport.IsValid())
    {
        const FGeometry& Geometry = Viewport->GetCachedGeometry();
        const FVector2D Clamped(FMath::Clamp(NormalizedPosition.X, 0.0f, 1.0f), FMath::Clamp(NormalizedPosition.Y, 0.0f, 1.0f));
        User->SetCursorPosition(Geometry.LocalToAbsolute(Geometry.GetLocalSize() * Clamped));
    }
}

EOrientation FCommonAnalogCursor::NativeGetScrollOrientation(const UWidget* Widget) const
{
    if (const UListView* List = Cast<UListView>(Widget))
        return List->GetOrientation();
    if (const UScrollBar* Bar = Cast<UScrollBar>(Widget))
        return Bar->Orientation;
    if (const UScrollBox* Box = Cast<UScrollBox>(Widget))
        return Box->Orientation;
    return Orient_Vertical;
}

void FCommonAnalogCursor::Tick(float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
    NativeUpdateCursorVisibility();
    if (CurrentInputMethod != ECommonInputType::Gamepad || !NativeShouldHandleInput())
        return;
    TSharedPtr<FSlateUser> User = SlateApp.GetUser(GetOwnerUserIndex());
    if (bIsGamepadCursor)
        FAnalogCursor::Tick(DeltaTime, SlateApp, Cursor);
    else
    {
        TSharedPtr<SWidget> PreviousTarget = LastCursorTarget.Pin();
        TSharedPtr<SWidget> Target = User->GetFocusedWidget();
        if (Target.IsValid())
            if (TSharedPtr<ITableViewMetadata> Table = Target->GetMetaData<ITableViewMetadata>())
            {
                const TArray<TSharedPtr<ITableRow>> Rows = Table->GatherSelectedRows();
                if (Rows.Num() > 0 && Rows[0].IsValid())
                    Target = Rows[0]->AsWidget();
            }

        if (Target != PreviousTarget || (Target.IsValid()
            && !CommonCursorTransformEquals(Target->GetCachedGeometry().GetAccumulatedRenderTransform(), NativeLastCursorTargetTransform)))
        {
            if (Target != PreviousTarget && User->HasCursorCapture() && !User->DoesWidgetHaveAnyCapture(Target))
                User->ReleaseCursorCapture();
            LastCursorTarget = Target;
            bool bHasUsableGeometry = false;
            if (Target.IsValid())
            {
                FGeometry Geometry;
                ULocalPlayer* Player = Router.GetLocalPlayer();
                UGameViewportClient* ViewportClient = Player->ViewportClient;
                if (Target == ViewportClient->GetGameViewportWidget())
                {
                    if (TSharedPtr<IGameLayerManager> Layers = ViewportClient->GetGameLayerManager())
                        Geometry = Layers->GetPlayerWidgetHostGeometry(Player);
                }
                else
                    Geometry = Target->GetCachedGeometry();
                NativeLastCursorTargetTransform = Geometry.GetAccumulatedRenderTransform();
                if (Geometry.GetLocalSize().SizeSquared() > 1.e-8f)
                {
                    User->SetCursorPosition(Geometry.LocalToAbsolute(Geometry.GetLocalSize() * 0.5f));
                    bHasUsableGeometry = true;
                }
            }
            if (!bHasUsableGeometry)
                NativeSetCursorPositionInViewport(FVector2D::ZeroVector);
        }
    }

    if (!bUnknownD6)
        return;
    NativeTimeUntilScrollUpdate -= DeltaTime;
    const FVector2D Analog = GetAnalogValues(EAnalogStick::Right);
    if (NativeTimeUntilScrollUpdate > 0.0f || Analog.SizeSquared() <= 0.04f || !Router.NativeActiveRouteRoot.IsValid())
        return;
    TArray<UWidget*> ScrollWidgets;
    Router.NativeActiveRouteRoot->CollectRoutedWidgets(ScrollWidgets);
    if (ScrollWidgets.Num() == 0)
        return;
    const FCommonAnalogCursorSettings& Settings = GetDefault<UCommonUIInputSettings>()->AnalogCursorSettings;
    // The original divides by DeadZone - 1, deliberately reversing the sign.
    const auto ScrollAmount = [&Settings](float Value)
    {
        return FMath::Abs(Value) > Settings.ScrollDeadZone
            ? (FMath::Abs(Value) - Settings.ScrollDeadZone) / (Settings.ScrollDeadZone - 1.0f)
                * FMath::Sign(Value) * Settings.ScrollMultiplier : 0.0f;
    };
    const float VerticalScroll = ScrollAmount(Analog.Y);
    const float HorizontalScroll = ScrollAmount(Analog.X);
    ULocalPlayer* Player = Router.GetLocalPlayer();
    TSharedPtr<SViewport> Viewport = Player->ViewportClient ? Player->ViewportClient->GetGameViewportWidget() : nullptr;
    for (UWidget* Widget : ScrollWidgets)
    {
        if (!Widget->GetCachedWidget().IsValid())
            continue;
        const float Amount = NativeGetScrollOrientation(Widget) == Orient_Vertical ? VerticalScroll : HorizontalScroll;
        if (FMath::Abs(Amount) <= 1.e-8f)
            continue;
        const FGeometry& Geometry = Widget->GetCachedGeometry();
        const FVector2D Position = Geometry.LocalToAbsolute(Geometry.GetLocalSize() * 0.5f);
        if (!Viewport.IsValid())
            continue;
        // Shipping uses the viewport's accumulated layout transform for bounds.
        const FGeometry& ViewportGeometry = Viewport->GetCachedGeometry();
        const FVector2D Min = TransformPoint(ViewportGeometry.GetAccumulatedLayoutTransform(), FVector2D::ZeroVector);
        const FVector2D Max = TransformPoint(ViewportGeometry.GetAccumulatedLayoutTransform(), ViewportGeometry.GetLocalSize());
        if (Position.X < Min.X || Position.X > Max.X || Position.Y < Min.Y || Position.Y > Max.Y)
            continue;
        const TSet<FKey> NoButtons;
        const FPointerEvent WheelEvent(User->GetUserIndex(), FSlateApplication::CursorPointerIndex,
            Position, Position, NoButtons, EKeys::Invalid, Amount, FModifierKeysState());
        SlateApp.ProcessMouseWheelOrGestureEvent(WheelEvent, nullptr);
    }
}
