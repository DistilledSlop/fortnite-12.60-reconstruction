#include "ButtonInternalProbe.h"
#include "ActivationBaseProbe.h"
#include "Blueprint/WidgetTree.h"
#include "../CommonUI/Private/SCommonButton.h"
#include "Widgets/Layout/SBox.h"
#include "UCommonUISubsystem.h"
#include "UCommonInputManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "UObject/StrongObjectPtr.h"
#include "InputCoreTypes.h"

namespace ButtonInternalProbeDiagnosticInternal {
int32 Sequence = 0;
bool Failed = false;
bool Registered = false;
FDelegateHandle FrameHandle;
double Deadline = 0;
void Record(const TCHAR* Event, bool OK = true)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("BTNINTERNAL v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
}
void Run(UWorld* World, UGameInstance* GI, ULocalPlayer* Player, APlayerController* PC, UCommonUISubsystem* Subsystem)
{
    Record(TEXT("begin"));
    TStrongObjectPtr<UActivationBaseProbeWidget> Owner(CreateWidget<UActivationBaseProbeWidget>(PC, UActivationBaseProbeWidget::StaticClass()));
    if (!Owner.IsValid() || !Owner->WidgetTree) { Record(TEXT("owner_missing"), false); return; }
    TStrongObjectPtr<UButtonInternalProbeWidget> Widget(NewObject<UButtonInternalProbeWidget>(Owner->WidgetTree));
    UButtonInternalProbeWidget* W = Widget.Get();
    auto* CDO = GetDefault<UButtonInternalProbeWidget>();
    Record(TEXT("defaults"), W && CDO && Owner->GetWorld() == World && W->GetWorld() == World && PC->Player == Player &&
        W->bButtonEnabled && W->bInteractionEnabled && CDO->bButtonEnabled && CDO->bInteractionEnabled &&
        !W->BaseButton().IsValid() && !W->CommonButton().IsValid() && !W->Box().IsValid());
    if (!W) return;
    W->NativeSetButtonEnabled(false);
    W->NativeSetInteractionEnabled(false);
    W->NativeSetInteractionEnabled(false);
    Record(TEXT("prebuild_disabled"), !W->bButtonEnabled && !W->bInteractionEnabled && !W->CommonButton().IsValid());
    W->NativeSetButtonEnabled(true);
    W->NativeSetInteractionEnabled(true);
    Record(TEXT("prebuild_enabled"), W->bButtonEnabled && W->bInteractionEnabled && !W->CommonButton().IsValid());
    W->MinWidth = 500; W->MinHeight = 600;
    TSharedPtr<SWidget> Returned = W->TakeWidget();
    TSharedPtr<SWidget> Box = W->Box();
    TSharedPtr<SButton> Child = W->CommonButton();
    Record(TEXT("factory"), Box.IsValid() && Child.IsValid() && Returned == Box &&
        W->BaseButton() == Child && Box->GetType() == FName(TEXT("SBox")) &&
        StaticCastSharedPtr<SBox>(Box)->GetChildren()->Num() == 1 &&
        &StaticCastSharedPtr<SBox>(Box)->GetChildren()->GetChildAt(0).Get() == Child.Get());
    if (!Box.IsValid() || !Child.IsValid()) return;
    Box->SlatePrepass(1.0f);
    FVector2D Size = Box->GetDesiredSize();
    Record(TEXT("size"), Size.X >= 500.0f && Size.Y >= 600.0f);
    FGeometry Geometry;
    FPointerEvent Pointer;
    Child->OnMouseEnter(Geometry, Pointer);
    Record(TEXT("hover_enabled"), Child->IsHovered());
    W->NativeSetInteractionEnabled(false);
    W->NativeSetInteractionEnabled(false);
    Record(TEXT("interaction_disabled"), !W->bInteractionEnabled && !Child->IsInteractable() && !Child->IsHovered() && !Child->IsPressed());
    Record(TEXT("disabled_mouse_down"), Child->OnMouseButtonDown(Geometry, Pointer).IsEventHandled());
    W->NativeSetInteractionEnabled(true);
    W->NativeSetInteractionEnabled(true);
    Record(TEXT("interaction_enabled"), W->bInteractionEnabled && Child->IsInteractable() && Child->IsHovered());
    W->NativeSetButtonEnabled(false);
    Record(TEXT("button_disabled"), !W->bButtonEnabled && !Child->IsInteractable() && Child->IsHovered() &&
        Child->OnMouseButtonDown(Geometry, Pointer).IsEventHandled());
    W->NativeSetButtonEnabled(true);
    Record(TEXT("button_enabled"), W->bButtonEnabled && Child->IsInteractable() && Child->IsHovered());
    Child->OnMouseLeave(Pointer);
    Record(TEXT("hover_leave"), !Child->IsHovered());
    FKeyEvent Enter(EKeys::Enter, FModifierKeysState(), 0, false, 0, 0);
    Record(TEXT("enter_down"), !Child->OnKeyDown(Geometry, Enter).IsEventHandled());
    Record(TEXT("enter_up"), !Child->OnKeyUp(Geometry, Enter).IsEventHandled());
    FKeyEvent Other(EKeys::A, FModifierKeysState(), 0, false, 0, 0);
    Record(TEXT("other_key"), !Child->OnKeyDown(Geometry, Other).IsEventHandled() && !Child->OnKeyUp(Geometry, Other).IsEventHandled());
    TStrongObjectPtr<UButtonInternalProbeSink> Sink(NewObject<UButtonInternalProbeSink>());
    TStrongObjectPtr<UCommonInputManager> OldManager(Subsystem->CommonInputManager);
    TStrongObjectPtr<UCommonInputManager> Manager(NewObject<UCommonInputManager>(Subsystem));
    Subsystem->CommonInputManager = Manager.Get();
    W->NativeDoubleClickDelegate.BindUObject(Sink.Get(), &UButtonInternalProbeSink::NativeDouble);
    W->NativeFocusReceivedDelegate.BindUObject(Sink.Get(), &UButtonInternalProbeSink::NativeFocus);
    W->OnDoubleClicked.AddDynamic(Sink.Get(), &UButtonInternalProbeSink::DynamicDouble);
    W->OnClicked.AddDynamic(Sink.Get(), &UButtonInternalProbeSink::DynamicClick);
    W->OnPressed.AddDynamic(Sink.Get(), &UButtonInternalProbeSink::DynamicPress);
    W->OnReleased.AddDynamic(Sink.Get(), &UButtonInternalProbeSink::DynamicRelease);
    Manager->NativeSetExternalInputSuspended(false);
    Record(TEXT("manager_active"), !Manager->IsInputSuspended());
    Record(TEXT("double_result"), Child->OnMouseButtonDoubleClick(Geometry, Pointer).IsEventHandled());
    Record(TEXT("click_result"), W->Click().IsEventHandled());
    W->Press(); W->Release();
    Record(TEXT("focus_result"), !Child->OnFocusReceived(Geometry, FFocusEvent(EFocusCause::SetDirectly, 0)).IsEventHandled());
    Manager->NativeSetExternalInputSuspended(true);
    Record(TEXT("manager_suspended"), Manager->IsInputSuspended());
    Record(TEXT("suspended_double"), !W->DoubleClick().IsEventHandled());
    Record(TEXT("suspended_click"), W->Click().IsEventHandled());
    W->Press(); W->Release();
    Manager->NativeSetExternalInputSuspended(false);
    Record(TEXT("manager_restored"), !Manager->IsInputSuspended());
    W->NativeDoubleClickDelegate.Unbind(); W->NativeFocusReceivedDelegate.Unbind();
    W->OnDoubleClicked.RemoveDynamic(Sink.Get(), &UButtonInternalProbeSink::DynamicDouble);
    W->OnClicked.RemoveDynamic(Sink.Get(), &UButtonInternalProbeSink::DynamicClick);
    W->OnPressed.RemoveDynamic(Sink.Get(), &UButtonInternalProbeSink::DynamicPress);
    W->OnReleased.RemoveDynamic(Sink.Get(), &UButtonInternalProbeSink::DynamicRelease);
    Subsystem->CommonInputManager = OldManager.Get();
    Record(TEXT("delegates_cleanup"), !W->NativeDoubleClickDelegate.IsBound() && !W->NativeFocusReceivedDelegate.IsBound() &&
        !W->OnDoubleClicked.IsBound() && !W->OnClicked.IsBound() && !W->OnPressed.IsBound() && !W->OnReleased.IsBound() &&
        Subsystem->CommonInputManager == OldManager.Get());
    TWeakPtr<SWidget> WeakBox = Box; TWeakPtr<SButton> WeakChild = Child;
    W->ReleaseProbe();
    Record(TEXT("owners_released"), !W->BaseButton().IsValid() && !W->CommonButton().IsValid() && !W->Box().IsValid() && WeakBox.IsValid() && WeakChild.IsValid());
    Returned.Reset(); Box.Reset(); Child.Reset();
    Record(TEXT("slate_destroyed"), !WeakBox.IsValid() && !WeakChild.IsValid());
}
void Finish(bool OK)
{
    FCoreDelegates::OnEndFrame.Remove(FrameHandle);
    Record(TEXT("complete"), OK && !Failed);
    FPlatformMisc::RequestExitWithStatus(false, Failed ? 1 : 0);
}
void Tick()
{
    if (FPlatformTime::Seconds() > Deadline) { Record(TEXT("timeout"), false); Finish(false); return; }
    if (!GEngine) return;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* W = Context.World(); UGameInstance* GI = W ? W->GetGameInstance() : nullptr;
        ULocalPlayer* P = GI ? GI->GetFirstGamePlayer() : nullptr;
        APlayerController* PC = P ? P->GetPlayerController(W) : nullptr;
        UCommonUISubsystem* S = GI ? GI->GetSubsystem<UCommonUISubsystem>() : nullptr;
        if (!W || !W->IsGameWorld() || !GI || !P || !PC || !S || PC->Player != P ||
            PC->GetWorld() != W || P->GetGameInstance() != GI || GI->GetWorld() != W) continue;
        Run(W, GI, P, PC, S); Finish(!Failed); return;
    }
}
}
TSharedPtr<SWidget> UButtonInternalProbeWidget::Box() const { return NativeButtonBox; }
TSharedPtr<SButton> UButtonInternalProbeWidget::CommonButton() const { return NativeCommonButton; }
FReply UButtonInternalProbeSink::NativeDouble() { ButtonInternalProbeDiagnosticInternal::Record(TEXT("native_double")); return FReply::Unhandled(); }
void UButtonInternalProbeSink::NativeFocus() { ButtonInternalProbeDiagnosticInternal::Record(TEXT("native_focus")); }
void UButtonInternalProbeSink::DynamicDouble() { ButtonInternalProbeDiagnosticInternal::Record(TEXT("dynamic_double")); }
void UButtonInternalProbeSink::DynamicClick() { ButtonInternalProbeDiagnosticInternal::Record(TEXT("dynamic_click")); }
void UButtonInternalProbeSink::DynamicPress() { ButtonInternalProbeDiagnosticInternal::Record(TEXT("dynamic_press")); }
void UButtonInternalProbeSink::DynamicRelease() { ButtonInternalProbeDiagnosticInternal::Record(TEXT("dynamic_release")); }
void RegisterButtonInternalProbeDriver()
{
    using namespace ButtonInternalProbeDiagnosticInternal;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("ButtonInternalProbe"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
