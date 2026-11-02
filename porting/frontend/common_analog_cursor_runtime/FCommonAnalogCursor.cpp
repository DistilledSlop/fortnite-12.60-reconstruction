#include "FCommonAnalogCursor.h"
#include "UCommonUIActionRouter.h"
#include "UCommonInputSubsystem.h"
#include "UCommonUIInputSettings.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Console.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "Layout/WidgetPath.h"
#include "Widgets/SViewport.h"

FCommonAnalogCursor::FCommonAnalogCursor(UCommonUIActionRouter& InRouter)
    : Router(InRouter)
{
}

TSharedRef<FCommonAnalogCursor> NativeCreateCommonAnalogCursor(UCommonUIActionRouter& Router)
{
    TSharedRef<FCommonAnalogCursor> Cursor = MakeShared<FCommonAnalogCursor>(Router);
    Cursor->Initialize();
    return Cursor;
}

void FCommonAnalogCursor::Initialize()
{
    const FCommonAnalogCursorSettings& Settings = GetDefault<UCommonUIInputSettings>()->AnalogCursorSettings;
    SetAcceleration(Settings.CursorAcceleration);
    SetMaxSpeed(Settings.CursorMaxSpeed);
    SetDeadZone(Settings.CursorDeadZone);
    SetStickySlowdown(Settings.HoverSlowdownFactor);
    SetMode(Settings.bEnableCursorAcceleration ? AnalogCursorMode::Accelerated : AnalogCursorMode::Direct);

    PressedMouseKeys = FSlateApplication::Get().GetPressedMouseButtons();
    PressedMouseKeys.Remove(EKeys::LeftMouseButton);
    PressedMouseKeys.Remove(EKeys::RightMouseButton);

    UCommonInputSubsystem* Input = Router.GetLocalPlayer()->GetSubsystem<UCommonInputSubsystem>();
    Input->NativeInputMethodChanged.AddSP(StaticCastSharedRef<FCommonAnalogCursor>(AsShared()), &FCommonAnalogCursor::HandleInputMethodChanged);
    HandleInputMethodChanged(Input->GetCurrentInputType());
}

void FCommonAnalogCursor::HandleInputMethodChanged(ECommonInputType InputType)
{
    CurrentInputMethod = InputType;
    if (InputType == ECommonInputType::Gamepad)
        LastCursorTarget.Reset();
}

TSharedRef<IInputProcessor> UCommonUIActionRouter::NativeCreateActionInputPreProcessor()
{
    return NativeCreateCommonAnalogCursor(*this);
}

bool FCommonAnalogCursor::NativeViewportBlocksKeyDown() const
{
    ULocalPlayer* Player = Router.GetLocalPlayer();
    UGameViewportClient* Viewport = Player ? Player->ViewportClient : nullptr;
    return Viewport && Viewport->ViewportConsole && Viewport->ViewportConsole->ConsoleActive();
}

int32 FCommonAnalogCursor::GetOwnerUserIndex() const
{
    ULocalPlayer* Player = Router.GetLocalPlayer();
    return Player && Player->GetGameInstance()
        ? Player->GetGameInstance()->GetLocalPlayers().IndexOfByKey(Player) : INDEX_NONE;
}

bool FCommonAnalogCursor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& Event)
{
    if (!IsRelevantKeyInput(Event) || NativeViewportBlocksKeyDown())
        return false;
    const bool bVirtualAccept = Event.GetKey() == EKeys::Virtual_Accept;
    if (bVirtualAccept && Router.NativeProcessActionInput(Event.GetKey(), Event.IsRepeat() ? IE_Repeat : IE_Pressed) == 0)
        return true;
    UCommonInputSubsystem* Input = Router.GetLocalPlayer()->GetSubsystem<UCommonInputSubsystem>();
    Input->bIsGamepadSimulatedClick = bVirtualAccept;
    const bool bHandled = FAnalogCursor::HandleKeyDownEvent(SlateApp, Event);
    Input->bIsGamepadSimulatedClick = false;
    return bHandled;
}

bool FCommonAnalogCursor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& Event)
{
    if (!IsRelevantKeyInput(Event))
        return false;
    if (Event.GetKey() == EKeys::Virtual_Accept && Router.NativeProcessActionInput(Event.GetKey(), IE_Released) == 0)
        return true;
    return FAnalogCursor::HandleKeyUpEvent(SlateApp, Event);
}

bool FCommonAnalogCursor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& Event)
{
    if (!IsRelevantInput(Event))
        return false;
    const bool bHandled = FAnalogCursor::HandleAnalogInputEvent(SlateApp, Event);
    return bIsGamepadCursor && bHandled;
}

bool FCommonAnalogCursor::HandleMouseMoveEvent(FSlateApplication&, const FPointerEvent&)
{
    return false;
}

static bool IsRoutableMouseButton(FKey Key)
{
    return Key.IsMouseButton() && Key != EKeys::LeftMouseButton && Key != EKeys::RightMouseButton;
}

bool FCommonAnalogCursor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& Event)
{
    const FKey Key = Event.GetEffectingButton();
    if (!FAnalogCursor::IsRelevantInput(Event) || !IsRoutableMouseButton(Key))
        return false;
    ULocalPlayer* Player = Router.GetLocalPlayer();
    TSharedPtr<SViewport> Viewport = Player && Player->ViewportClient ? Player->ViewportClient->GetGameViewportWidget() : nullptr;
    if (!Viewport.IsValid())
        return false;
    const FWidgetPath Path = SlateApp.LocateWindowUnderMouse(Event.GetScreenSpacePosition(), SlateApp.GetInteractiveTopLevelWindows(), false, INDEX_NONE);
    if (!Path.ContainsWidget(Viewport.ToSharedRef()))
        return false;
    const FKeyEvent KeyEvent(Key, Event.GetModifierKeys(), Event.GetUserIndex(), false, 0, 0);
    if (!SlateApp.ProcessKeyDownEvent(KeyEvent))
        return false;
    PressedMouseKeys.Add(Key);
    return true;
}

bool FCommonAnalogCursor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& Event)
{
    if (!FAnalogCursor::IsRelevantInput(Event))
        return false;
    const FKey Key = Event.GetEffectingButton();
    const bool bWasPressed = PressedMouseKeys.Remove(Key) > 0;
    if (!bWasPressed && (!IsRoutableMouseButton(Key) || SlateApp.HasUserMouseCapture(Event.GetUserIndex())))
        return false;
    const FKeyEvent KeyEvent(Key, Event.GetModifierKeys(), Event.GetUserIndex(), false, 0, 0);
    const bool bHandled = SlateApp.ProcessKeyUpEvent(KeyEvent);
    return bWasPressed && bHandled;
}
