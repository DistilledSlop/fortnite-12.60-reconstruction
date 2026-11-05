#include "RootWidgetInstanceProbe.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
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
#include "UObject/UObjectGlobals.h"

namespace RootWidgetInstanceProbeInternal
{
FString RootPath = TEXT("/Game/UI/Foundation/RootLayout_Athena.RootLayout_Athena_C");
FDelegateHandle FrameHandle;
FDelegateHandle ExitHandle;
TStrongObjectPtr<UUserWidget> Instance;
double Deadline = 0;
bool Registered = false;
bool Complete = false;
bool AttachRequested = false;
bool Attached = false;
bool ViewOK = false;
int32 FrameIndex = 0;
float CaptureSeconds = 0.0f;
double CaptureUntil = 0.0;

void LogView(const TCHAR* Phase, UUserWidget* Widget)
{
    UE_LOG(LogTemp, Display, TEXT("ROOTVIEW v=1 phase=%s in_viewport=%d cached_widget=%d"),
        Phase, int32(Widget->IsInViewport()), int32(Widget->GetCachedWidget().IsValid()));
}

void Finish(bool OK)
{
    Complete = true;
    FCoreDelegates::OnEndFrame.Remove(FrameHandle);
    FCoreDelegates::OnEnginePreExit.Remove(ExitHandle);
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=2 phase=complete ok=%d"), int32(OK));
    Instance.Reset();
    FPlatformMisc::RequestExitWithStatus(false, OK ? 0 : 1);
}

void OnEnginePreExit()
{
    if (!Complete)
    {
        UE_LOG(LogTemp, Error, TEXT("ROOTINSTANCE v=2 phase=engine_exit_before_completion ok=0"));
        FPlatformMisc::RequestExitWithStatus(false, 1);
    }
}

void Run(UWorld* World, UGameInstance* GameInstance, ULocalPlayer* Player, APlayerController* Controller)
{
    UGameViewportClient* Viewport = GameInstance->GetGameViewportClient();
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=2 phase=prerequisites world=%s game_instance=%s player=%s controller=%s game_instance_class=%s player_class=%s controller_class=%s viewport_class=%s"),
        *GetPathNameSafe(World), *GetPathNameSafe(GameInstance), *GetPathNameSafe(Player), *GetPathNameSafe(Controller),
        *GetPathNameSafe(GameInstance->GetClass()), *GetPathNameSafe(Player->GetClass()),
        *GetPathNameSafe(Controller->GetClass()), *GetPathNameSafe(Viewport ? Viewport->GetClass() : nullptr));
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=2 phase=before_load path=%s"), *RootPath);
    UClass* RootClass = LoadClass<UUserWidget>(nullptr, *RootPath);
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=2 phase=after_load class=%s ok=%d"),
        *GetPathNameSafe(RootClass), int32(RootClass != nullptr));
    if (!RootClass) { Finish(false); return; }

    // UE 4.25 CreateWidget calls Initialize internally; its return value is not exposed.
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=2 phase=before_create_widget"));
    Instance.Reset(CreateWidget<UUserWidget>(Controller, RootClass));
    UUserWidget* Widget = Instance.Get();
    TArray<UWidget*> Widgets;
    if (Widget && Widget->WidgetTree) Widget->WidgetTree->GetAllWidgets(Widgets);
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=2 phase=after_create_widget widget=%s class=%s outer=%s world=%s player=%s controller=%s tree=%s root_widget=%s widget_count=%d"),
        *GetPathNameSafe(Widget), *GetPathNameSafe(Widget ? Widget->GetClass() : nullptr),
        *GetPathNameSafe(Widget ? Widget->GetOuter() : nullptr), *GetPathNameSafe(Widget ? Widget->GetWorld() : nullptr),
        *GetPathNameSafe(Widget ? Widget->GetOwningLocalPlayer() : nullptr),
        *GetPathNameSafe(Widget ? Widget->GetOwningPlayer() : nullptr),
        *GetPathNameSafe(Widget ? Widget->WidgetTree : nullptr),
        *GetPathNameSafe(Widget && Widget->WidgetTree ? Widget->WidgetTree->RootWidget : nullptr), Widgets.Num());
    const bool OK = Widget && Widget->GetClass() == RootClass && Widget->GetOuter() == GameInstance &&
        Widget->GetWorld() == World && Widget->GetOwningLocalPlayer() == Player &&
        Widget->GetOwningPlayer() == Controller && Widget->WidgetTree;
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=2 phase=observations ok=%d scope=create_return_and_ownership"), int32(OK));
    if (!AttachRequested || !OK) { Finish(OK); return; }

    ViewOK = OK && !Widget->IsInViewport();
    LogView(TEXT("before_attach"), Widget);
    Widget->AddToViewport(0);
    LogView(TEXT("after_attach"), Widget);
    ViewOK = ViewOK && Widget->IsInViewport() && Widget->GetCachedWidget().IsValid();
    Attached = true;
    CaptureUntil = FPlatformTime::Seconds() + CaptureSeconds;
    Deadline = FPlatformTime::Seconds() + 180.0 + CaptureSeconds;
}

void Tick()
{
    if (FPlatformTime::Seconds() >= Deadline)
    {
        if (Attached)
        {
            UE_LOG(LogTemp, Error, TEXT("ROOTVIEW v=1 phase=timeout ok=0"));
            UE_LOG(LogTemp, Display, TEXT("ROOTVIEW v=1 phase=complete ok=0"));
            Finish(false);
            return;
        }
        UE_LOG(LogTemp, Error, TEXT("ROOTINSTANCE v=2 phase=prerequisites_timeout ok=0"));
        Finish(false);
        return;
    }
    if (Attached)
    {
        UUserWidget* Widget = Instance.Get();
        ++FrameIndex;
        if (FrameIndex <= 3)
        {
            UE_LOG(LogTemp, Display, TEXT("ROOTVIEW v=1 phase=frame index=%d engine_frame=%llu in_viewport=%d cached_widget=%d"),
                FrameIndex, (unsigned long long)GFrameCounter, int32(Widget->IsInViewport()), int32(Widget->GetCachedWidget().IsValid()));
        }
        ViewOK = ViewOK && Widget->IsInViewport() && Widget->GetCachedWidget().IsValid();
        if (FrameIndex >= 3 && FPlatformTime::Seconds() >= CaptureUntil)
        {
            LogView(TEXT("before_remove"), Widget);
            Widget->RemoveFromParent();
            LogView(TEXT("after_remove"), Widget);
            ViewOK = ViewOK && !Widget->IsInViewport();
            UE_LOG(LogTemp, Display, TEXT("ROOTVIEW v=1 phase=complete ok=%d"), int32(ViewOK));
            Finish(ViewOK);
        }
        return;
    }
    if (!GEngine) return;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* World = Context.World();
        UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
        ULocalPlayer* Player = GameInstance ? GameInstance->GetFirstGamePlayer() : nullptr;
        APlayerController* Controller = Player ? Player->GetPlayerController(World) : nullptr;
        if (!World || !World->IsGameWorld() || World->bIsTearingDown || !GameInstance || !Player || !Controller ||
            Controller->Player != Player || !Controller->IsLocalPlayerController() || Controller->GetWorld() != World ||
            Player->GetGameInstance() != GameInstance || GameInstance->GetWorld() != World) continue;
        Run(World, GameInstance, Player, Controller);
        return;
    }
}
}

void RegisterRootWidgetInstanceProbeDriver()
{
    using namespace RootWidgetInstanceProbeInternal;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("RootWidgetInstanceProbe"))) return;
    Registered = true;
    AttachRequested = FParse::Param(FCommandLine::Get(), TEXT("RootWidgetInstanceAttach"));
    // Capture controls are opt-in diagnostics; ordinary startup is unchanged.
    FParse::Value(FCommandLine::Get(), TEXT("RootWidgetInstanceClass="), RootPath);
    FParse::Value(FCommandLine::Get(), TEXT("RootWidgetCaptureSeconds="), CaptureSeconds);
    CaptureSeconds = FMath::Clamp(CaptureSeconds, 0.0f, 300.0f);
    Deadline = FPlatformTime::Seconds() + 180.0 + CaptureSeconds;
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=2 phase=registered timeout_seconds=180"));
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
    ExitHandle = FCoreDelegates::OnEnginePreExit.AddStatic(&OnEnginePreExit);
}
