#include "CommonButtonProductionFlowProbe.h"
#include "CommonPanelActionNative.h"
#include "UCommonButtonInternal.h"
#include "UCommonInputSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ButtonSlot.h"
#include "Components/TextBlock.h"
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
#include "UObject/GarbageCollection.h"
#include "UObject/StrongObjectPtr.h"

void UButtonFlowWidget::NativeOnInitialized()
{
    ++Initialized; bWeakAbsentAtInitialized = !Internal();
    Super::NativeOnInitialized();
}
void UButtonFlowWidget::NativeOnClicked() { ++Clicks; Super::NativeOnClicked(); }
void UButtonFlowWidget::NativeOnDoubleClicked() { ++Doubles; Super::NativeOnDoubleClicked(); }
void UButtonFlowWidget::NativeOnCurrentTextStyleChanged()
{
    ++TextEvents;
    if (bReenterStyle) { bReenterStyle = false; SetStyle(UButtonFlowStyleB::StaticClass()); }
    Super::NativeOnCurrentTextStyleChanged();
}

namespace ButtonFlowDiagnostic
{
int32 Sequence = 0;
bool Failed = false, Registered = false;
double Deadline = 0;
FDelegateHandle FrameHandle;
void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("BUTTONFLOW v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
}
UButtonFlowWidget* Make(UCommonUserWidget* Owner, bool Root)
{
    UButtonFlowWidget* Widget = NewObject<UButtonFlowWidget>(Owner->WidgetTree);
    Widget->WidgetTree = NewObject<UWidgetTree>(Widget, TEXT("WidgetTree"), RF_Transient);
    if (Root) Widget->WidgetTree->RootWidget = Widget->WidgetTree->ConstructWidget<UTextBlock>();
    Widget->Configure();
    return Widget;
}
void Run(ULocalPlayer* Player, APlayerController* PC)
{
    Record(TEXT("begin"), true);
    TStrongObjectPtr<UCommonUserWidget> Owner(CreateWidget<UCommonUserWidget>(PC, UCommonUserWidget::StaticClass()));
    UCommonInputSubsystem* Input = Player->GetSubsystem<UCommonInputSubsystem>();
    Record(TEXT("context"), Owner.IsValid() && Owner->WidgetTree && Input && Owner->GetWorld() == PC->GetWorld());
    if (!Owner.IsValid() || !Owner->WidgetTree || !Input) return;
    TStrongObjectPtr<UButtonFlowWidget> Empty(Make(Owner.Get(), false));
    bool EmptyFirst = Empty->Initialize();
    TWeakObjectPtr<UCommonButtonInternal> EmptyWeak = Empty->Internal();
    Record(TEXT("empty_initialized"), EmptyFirst && Empty->Initialized == 1 && Empty->bWeakAbsentAtInitialized && EmptyWeak.IsValid() && !Empty->WidgetTree->RootWidget);
    Record(TEXT("empty_unbound"), EmptyWeak.IsValid() && !EmptyWeak->OnClicked.IsBound() && !EmptyWeak->OnPressed.IsBound()
        && !EmptyWeak->OnReleased.IsBound() && !EmptyWeak->NativeDoubleClickDelegate.IsBound() && !EmptyWeak->NativeFocusReceivedDelegate.IsBound());
    Record(TEXT("one_shot_empty"), !Empty->Initialize() && Empty->Initialized == 1 && Empty->Internal() == EmptyWeak.Get());
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    Record(TEXT("empty_weak_gc"), !EmptyWeak.IsValid() && !Empty->Internal());
    TStrongObjectPtr<UButtonFlowWidget> Button(Make(Owner.Get(), true));
    UWidget* OldRoot = Button->WidgetTree->RootWidget;
    bool First = Button->Initialize();
    UCommonButtonInternal* Internal = Button->Internal();
    UButtonSlot* Slot = Internal ? Cast<UButtonSlot>(OldRoot->Slot) : nullptr;
    Record(TEXT("root_initialized"), First && Button->Initialized == 1 && Button->bWeakAbsentAtInitialized && Internal
        && Button->WidgetTree->RootWidget == Internal && Internal->GetContent() == OldRoot);
    if (!Internal) return;
    Record(TEXT("root_slot"), Slot && Slot->Parent == Internal && Slot->Padding == FMargin(0)
        && Slot->HorizontalAlignment == HAlign_Fill && Slot->VerticalAlignment == VAlign_Fill);
    Record(TEXT("copied_fields"), Internal->ClickMethod == EButtonClickMethod::MouseDown
        && Internal->TouchMethod == EButtonTouchMethod::PreciseTap && Internal->PressMethod == EButtonPressMethod::ButtonPress
        && Internal->IsFocusable && Internal->bButtonEnabled && Internal->bInteractionEnabled);
    Record(TEXT("five_bindings"), Internal->OnClicked.IsBound() && Internal->OnPressed.IsBound() && Internal->OnReleased.IsBound()
        && Internal->NativeDoubleClickDelegate.IsBound() && Internal->NativeFocusReceivedDelegate.IsBound());
    Record(TEXT("one_shot_root"), !Button->Initialize() && Button->Internal() == Internal && Button->Initialized == 1);
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    Record(TEXT("root_gc"), Button->Internal() == Internal && Button->WidgetTree->RootWidget == Internal && Internal->GetContent() == OldRoot);
    if (Button->Internal() != Internal) return;
    UButtonFlowStyleA* A = GetMutableDefault<UButtonFlowStyleA>();
    UButtonFlowStyleB* B = GetMutableDefault<UButtonFlowStyleB>();
    const int32 AW = A->MinWidth, AH = A->MinHeight, BW = B->MinWidth, BH = B->MinHeight;
    A->MinWidth = 71; A->MinHeight = 31; B->MinWidth = 89; B->MinHeight = 47;
    Button->MinWidth = 73; Button->MinHeight = 20;
    Button->SetStyle(UButtonFlowStyleA::StaticClass());
    Record(TEXT("style_dimensions"), Button->Style == UButtonFlowStyleA::StaticClass() && Internal->MinWidth == 73 && Internal->MinHeight == 31 && Button->TextEvents > 0);
    int32 Before = Button->TextEvents;
    Button->SetStyle(nullptr); Button->SetStyle(UButtonFlowStyleA::StaticClass());
    Record(TEXT("style_null_equal"), Button->Style == UButtonFlowStyleA::StaticClass() && Button->TextEvents == Before);
    UFunction* Setter = Button->FindFunction(TEXT("SetStyle"));
    struct { UClass* InStyle; } BadParams { UTextBlock::StaticClass() };
    if (Setter) Button->ProcessEvent(Setter, &BadParams);
    Record(TEXT("style_wrong_class"), Setter && Button->Style == UButtonFlowStyleA::StaticClass() && Button->TextEvents == Before);
    Button->bReenterStyle = true;
    Button->SetStyle(UButtonFlowStyleB::StaticClass());
    // Trigger reentry with a different outer style; inner setter changes back to B.
    Button->bReenterStyle = true; Button->SetStyle(UButtonFlowStyleA::StaticClass());
    Record(TEXT("style_reentry"), !Button->bReenterStyle && Button->Style == UButtonFlowStyleB::StaticClass()
        && Internal->MinWidth == 89 && Internal->MinHeight == 47 && Button->TextEvents >= Before + 3);
    Button->bSelectable = true; Button->bToggleable = true; Button->bShouldSelectUponReceivingFocus = true;
    int32 Focus = 0, Selected = 0, Hover = 0, Unhover = 0;
    Button->NativeFocusReceived.AddLambda([&] { ++Focus; });
    Button->NativeSelectedChanged.AddLambda([&](bool Value) { Selected += Value ? 1 : 10; });
    Button->NativeHovered.AddLambda([&] { ++Hover; }); Button->NativeUnhovered.AddLambda([&] { ++Unhover; });
    Internal->NativeFocusReceivedDelegate.ExecuteIfBound();
    Record(TEXT("focus_selection"), Focus == 1 && Button->Selected() && Selected == 1);
    Internal->OnClicked.Broadcast();
    Record(TEXT("click_toggle"), Button->Clicks == 1 && !Button->Selected() && Selected == 11);
    Button->SetVisibility(ESlateVisibility::Collapsed); Internal->OnClicked.Broadcast();
    Record(TEXT("click_rejected"), Button->Clicks == 1 && !Button->Selected());
    Button->SetVisibility(ESlateVisibility::Visible);
    FReply Reply = Internal->NativeDoubleClickDelegate.IsBound() ? Internal->NativeDoubleClickDelegate.Execute() : FReply::Handled();
    Record(TEXT("double_click"), Button->Doubles == 1 && !Reply.IsEventHandled());
    const auto SavedType = Input->CurrentInputType;
    Input->CurrentInputType = ECommonInputType::Touch;
    Internal->OnPressed.Broadcast(); Internal->OnReleased.Broadcast();
    Record(TEXT("touch_press_release"), Hover == 1 && Unhover == 1);
    Input->CurrentInputType = SavedType;
    A->MinWidth = AW; A->MinHeight = AH; B->MinWidth = BW; B->MinHeight = BH;
    FCommonPanelActionRecord RawRecord;
    uint8 ObservedByte = 0;
    RawRecord.NativeCommit.BindLambda([&ObservedByte](bool& Value)
    {
        FMemory::Memcpy(&ObservedByte, &Value, sizeof(ObservedByte));
        Value = false;
    });
    bool RawValue;
    const uint8 SlotByte = 2;
    static_assert(sizeof(RawValue) == sizeof(SlotByte), "one-byte original bool ABI");
    FMemory::Memcpy(&RawValue, &SlotByte, sizeof(SlotByte));
    RawRecord.NativeCommit.Execute(RawValue);
    uint8 ResultByte = 255;
    FMemory::Memcpy(&ResultByte, &RawValue, sizeof(ResultByte));
    Record(TEXT("raw_commit_byte"), ObservedByte == 2 && ResultByte == 0);
    Record(TEXT("restored"), Input->CurrentInputType == SavedType && A->MinWidth == AW && B->MinHeight == BH);
}
void Finish()
{
    FCoreDelegates::OnEndFrame.Remove(FrameHandle);
    Record(TEXT("complete"), !Failed);
    FPlatformMisc::RequestExitWithStatus(false, Failed ? 1 : 0);
}
void Tick()
{
    if (FPlatformTime::Seconds() > Deadline) { Record(TEXT("timeout"), false); Finish(); return; }
    if (!GEngine) return;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* World = Context.World(); UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
        ULocalPlayer* Player = GI ? GI->GetFirstGamePlayer() : nullptr;
        APlayerController* PC = Player ? Player->GetPlayerController(World) : nullptr;
        if (!World || !World->IsGameWorld() || !Player || !PC || PC->Player != Player || PC->GetWorld() != World
            || Player->GetGameInstance() != GI || GI->GetWorld() != World) continue;
        Run(Player, PC); Finish(); return;
    }
}
}
void RegisterCommonButtonProductionFlowProbeDriver()
{
    using namespace ButtonFlowDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("CommonButtonProductionFlowProbe"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
