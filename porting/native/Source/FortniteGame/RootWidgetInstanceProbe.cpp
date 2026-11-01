#include "RootWidgetInstanceProbe.h"
#include "Blueprint/UserWidget.h"
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
#include "UObject/UObjectGlobals.h"

namespace RootWidgetInstanceProbeInternal
{
const TCHAR* const RootPath = TEXT("/Game/UI/Foundation/RootLayout_Athena.RootLayout_Athena_C");
FDelegateHandle FrameHandle;
FDelegateHandle ExitHandle;
TStrongObjectPtr<UUserWidget> Instance;
double Deadline = 0;
bool Registered = false;
bool Complete = false;

void Finish(bool OK)
{
    Complete = true;
    FCoreDelegates::OnEndFrame.Remove(FrameHandle);
    FCoreDelegates::OnEnginePreExit.Remove(ExitHandle);
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=1 phase=complete ok=%d"), int32(OK));
    Instance.Reset();
    FPlatformMisc::RequestExitWithStatus(false, OK ? 0 : 1);
}

void OnEnginePreExit()
{
    if (!Complete)
    {
        UE_LOG(LogTemp, Error, TEXT("ROOTINSTANCE v=1 phase=engine_exit_before_completion ok=0"));
        FPlatformMisc::RequestExitWithStatus(false, 1);
    }
}

void Run(UWorld* World, UGameInstance* GameInstance, ULocalPlayer* Player, APlayerController* Controller)
{
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=1 phase=prerequisites world=%s game_instance=%s player=%s controller=%s"),
        *GetPathNameSafe(World), *GetPathNameSafe(GameInstance), *GetPathNameSafe(Player), *GetPathNameSafe(Controller));
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=1 phase=before_load path=%s"), RootPath);
    UClass* RootClass = LoadClass<UUserWidget>(nullptr, RootPath);
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=1 phase=after_load class=%s ok=%d"),
        *GetPathNameSafe(RootClass), int32(RootClass != nullptr));
    if (!RootClass) { Finish(false); return; }

    // UE 4.25 CreateWidget calls Initialize internally; its return value is not exposed.
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=1 phase=before_create_widget"));
    Instance.Reset(CreateWidget<UUserWidget>(Controller, RootClass));
    UUserWidget* Widget = Instance.Get();
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=1 phase=after_create_widget widget=%s class=%s outer=%s world=%s player=%s controller=%s tree=%s"),
        *GetPathNameSafe(Widget), *GetPathNameSafe(Widget ? Widget->GetClass() : nullptr),
        *GetPathNameSafe(Widget ? Widget->GetOuter() : nullptr), *GetPathNameSafe(Widget ? Widget->GetWorld() : nullptr),
        *GetPathNameSafe(Widget ? Widget->GetOwningLocalPlayer() : nullptr),
        *GetPathNameSafe(Widget ? Widget->GetOwningPlayer() : nullptr),
        *GetPathNameSafe(Widget ? Widget->WidgetTree : nullptr));
    const bool OK = Widget && Widget->GetClass() == RootClass && Widget->GetOuter() == GameInstance &&
        Widget->GetWorld() == World && Widget->GetOwningLocalPlayer() == Player &&
        Widget->GetOwningPlayer() == Controller && Widget->WidgetTree;
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=1 phase=observations ok=%d scope=create_return_and_ownership"), int32(OK));
    Finish(OK);
}

void Tick()
{
    if (FPlatformTime::Seconds() >= Deadline)
    {
        UE_LOG(LogTemp, Error, TEXT("ROOTINSTANCE v=1 phase=prerequisites_timeout ok=0"));
        Finish(false);
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
    Deadline = FPlatformTime::Seconds() + 180.0;
    UE_LOG(LogTemp, Display, TEXT("ROOTINSTANCE v=1 phase=registered timeout_seconds=180"));
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
    ExitHandle = FCoreDelegates::OnEnginePreExit.AddStatic(&OnEnginePreExit);
}
