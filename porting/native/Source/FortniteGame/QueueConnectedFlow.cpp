#include "QueueConnectedFlow.h"
#include "UCommonActivatableWidgetQueue.h"
#include "UCommonUserWidget.h"
#include "SCommonAnimatedSwitcher.h"
#include "Blueprint/WidgetTree.h"
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
#include "Widgets/Layout/SBox.h"

TSharedRef<SWidget> UQueueConnectedFlowWidget::RebuildWidget()
{
    Super::RebuildWidget();
    return SNew(SBox).WidthOverride(32).HeightOverride(32);
}

namespace QueueConnectedFlow
{
int32 Sequence = 0, Stage = 0, Frame = 0, StageFrame = 0;
bool Failed = false, Registered = false;
double Deadline = 0, StageTime = 0;
bool SawDeferred = false;
FDelegateHandle FrameHandle;
TStrongObjectPtr<UCommonUserWidget> Owner;
TStrongObjectPtr<UCommonActivatableWidgetQueue> Queue;
TWeakObjectPtr<UCommonActivatableWidget> First, Second;
TWeakPtr<SWidget> RemovedSlate;
TArray<UCommonActivatableWidget*> DisplayEvents;

void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("QUEUEFLOW v=1 seq=%d frame=%d event=%s ok=%d"), ++Sequence, Frame, Event, int32(OK));
}
void Finish()
{
    if (Queue.IsValid())
    {
        Queue->NativeDisplayedWidgetChanged1f0.RemoveAll(Queue.Get());
        Queue->ReleaseSlateResources(true);
        Record(TEXT("teardown"), !Queue->NativeHasSwitcher1c8());
    }
    if (Owner.IsValid()) Owner->RemoveFromParent();
    Queue.Reset(); Owner.Reset();
    FCoreDelegates::OnEndFrame.Remove(FrameHandle);
    Record(TEXT("complete"), !Failed);
    FPlatformMisc::RequestExitWithStatus(false, Failed ? 1 : 0);
}
void Next(int32 NewStage) { Stage = NewStage; StageFrame = Frame; StageTime = FPlatformTime::Seconds(); }
void Begin(APlayerController* PC)
{
    Owner.Reset(CreateWidget<UCommonUserWidget>(PC, UCommonUserWidget::StaticClass()));
    Queue.Reset(Owner.IsValid() && Owner->WidgetTree ? Owner->WidgetTree->ConstructWidget<UCommonActivatableWidgetQueue>() : nullptr);
    Record(TEXT("context"), Owner.IsValid() && Queue.IsValid());
    if (!Queue.IsValid()) { Finish(); return; }
    Owner->WidgetTree->RootWidget = Queue.Get();
    Queue->TransitionDuration = 0.1f;
    Owner->AddToViewport();
    Record(TEXT("switcher_ready"), Queue->NativeHasSwitcher1c8() && Queue->NativeSwitcherIndex1407c19a0() == 0);
    if (!Queue->NativeHasSwitcher1c8()) { Finish(); return; }
    Queue->NativeDisplayedWidgetChanged1f0.AddWeakLambda(Queue.Get(), [](UCommonActivatableWidget* Widget) {
        DisplayEvents.Add(Widget);
        if (Widget == Second.Get() && Queue->NativeDeferredSlateWidgets1d8.Num())
        {
            SawDeferred = true;
            RemovedSlate = Queue->NativeDeferredSlateWidgets1d8[0];
        }
    });
    bool InitializedBeforeRegistration = false;
    UCommonActivatableWidget* A = Queue->NativeAddWidget1407aa4c0(UQueueConnectedFlowWidget::StaticClass(),
        [&](UCommonActivatableWidget* Widget) {
            InitializedBeforeRegistration = Widget && !Queue->WidgetList.Contains(Widget) && !Widget->bIsActive;
        });
    First = A;
    Record(TEXT("first_initializer"), A && InitializedBeforeRegistration);
    Record(TEXT("first_displayed"), A && Queue->WidgetList.Num() == 1 && Queue->WidgetList.Contains(A)
        && Queue->DisplayedWidget == A && A->bIsActive && DisplayEvents.Num() == 1 && DisplayEvents[0] == A);
    bool SecondInitialized = false;
    UCommonActivatableWidget* B = Queue->NativeAddWidget1407aa4c0(UQueueConnectedFlowWidget::StaticClass(),
        [&](UCommonActivatableWidget* Widget) { SecondInitialized = Widget && Widget != A && !Queue->WidgetList.Contains(Widget); });
    Second = B;
    Record(TEXT("second_queued"), B && SecondInitialized && Queue->WidgetList.Num() == 2
        && Queue->WidgetList.Contains(B) && Queue->DisplayedWidget == A && !B->bIsActive);
    if (!A || !B) { Finish(); return; }
    Next(1);
}
void Tick()
{
    ++Frame;
    if (FPlatformTime::Seconds() > Deadline) { Record(TEXT("timeout"), false); Finish(); return; }
    if (Stage == 0)
    {
        if (!GEngine) return;
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            UWorld* World = Context.World(); UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
            ULocalPlayer* Player = GI ? GI->GetFirstGamePlayer() : nullptr;
            APlayerController* PC = Player ? Player->GetPlayerController(World) : nullptr;
            if (World && World->IsGameWorld() && Player && PC && PC->Player == Player && PC->GetWorld() == World
                && Player->GetGameInstance() == GI && GI->GetWorld() == World) { Begin(PC); return; }
        }
        return;
    }
    if (!Queue.IsValid() || !First.IsValid() || !Second.IsValid()) { Record(TEXT("ownership"), false); Finish(); return; }
    if (Stage == 1 && Frame > StageFrame && FPlatformTime::Seconds() - StageTime > 0.3)
    {
        Record(TEXT("first_stays_active"), Queue->DisplayedWidget == First.Get() && First->bIsActive && !Second->bIsActive);
        Queue->NativeRemoveWidget1407f1000(First.Get());
        Record(TEXT("remove_deactivates"), !First->bIsActive);
        Next(2);
    }
    else if (Stage == 2 && Queue->DisplayedWidget == Second.Get() && Second->bIsActive)
    {
        Record(TEXT("next_displayed"), !Queue->WidgetList.Contains(First.Get()) && Queue->WidgetList.Contains(Second.Get())
            && DisplayEvents.Num() >= 2 && DisplayEvents.Last() == Second.Get());
        Record(TEXT("deferred_slate"), SawDeferred);
        Next(3);
    }
    else if (Stage == 3 && Frame > StageFrame + 1)
    {
        Record(TEXT("deferred_released"), Queue->NativeDeferredSlateWidgets1d8.Num() == 0 && !RemovedSlate.IsValid());
        Queue->ClearWidgets();
        Next(4);
    }
    else if (Stage == 4 && !Queue->DisplayedWidget && Queue->WidgetList.Num() == 0)
    {
        Record(TEXT("clear_empty"), !Second->bIsActive && Queue->NativeSwitcherIndex1407c19a0() == 0
            && DisplayEvents.Num() >= 3 && DisplayEvents.Last() == nullptr);
        bool Reinitialized = false;
        UCommonActivatableWidget* Reused = Queue->NativeAddWidget1407aa4c0(UQueueConnectedFlowWidget::StaticClass(),
            [&](UCommonActivatableWidget* Widget) { Reinitialized = Widget && !Queue->WidgetList.Contains(Widget); });
        Record(TEXT("pool_reuse"), Reused && (Reused == First.Get() || Reused == Second.Get()) && Reinitialized
            && Queue->DisplayedWidget == Reused && Reused->bIsActive);
        Queue->ClearWidgets();
        Next(5);
    }
    else if (Stage == 5 && !Queue->DisplayedWidget && Queue->WidgetList.Num() == 0)
    {
        Record(TEXT("reuse_clear"), Queue->NativeSwitcherIndex1407c19a0() == 0);
        Finish();
    }
}
}
void RegisterQueueConnectedFlowDriver()
{
    using namespace QueueConnectedFlow;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("QueueConnectedFlow"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
