// Diagnostic only. Compile into FortniteGame only for an isolated, disposable run.
#include "BlueprintContextRuntime.h"
#include "UBlueprintContextLibrary.h"
#include "UBlueprintContextBase.h"
#include "UFortReplayContext.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CoreDelegates.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UObject/GarbageCollection.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UnrealType.h"

namespace {
bool bRan = false;
void Check(const TCHAR* Name, bool Value)
{
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_PROBE_V1 %s %s"), Name, Value ? TEXT("PASS") : TEXT("FAIL"));
}
// Original ConstructFProperties class/object cases OR these exact pointer bits.
static_assert((uint64(CPF_IsPlainOldData) | uint64(CPF_NoDestructor) | uint64(CPF_ZeroConstructor) | uint64(CPF_HasGetValueTypeHash)) == 0x0008001040000200ULL, "original object property computed flag constants");
// Current C++ pointer type traits must equal the source-proven original OR bits.
struct FContextProbeObjectFlags final : TPropertyTypeFundamentals<UObject*>
{
    static uint64 Get() { return uint64(GetComputedFlagsPropertyFlags()); }
};
void LogContextReflectionProperty(UFunction* Function,const TCHAR* Name,uint64 Raw,uint64 Computed,
                                  int32 Offset,const TCHAR* Kind,UClass* PropertyClass,UClass* MetaClass)
{
    FProperty* P=Function ? FindFProperty<FProperty>(Function,Name) : nullptr;
    const FObjectPropertyBase* Object=CastField<FObjectPropertyBase>(P);
    const FClassProperty* Class=CastField<FClassProperty>(P);
    const FString ActualKind=P ? P->GetClass()->GetName() : TEXT("-");
    const FString ActualClass=Object && Object->PropertyClass ? Object->PropertyClass->GetPathName() : TEXT("-");
    const FString ActualMeta=Class && Class->MetaClass ? Class->MetaClass->GetPathName() : TEXT("-");
    const FString ExpectedClass=PropertyClass ? PropertyClass->GetPathName() : TEXT("-");
    const FString ExpectedMeta=MetaClass ? MetaClass->GetPathName() : TEXT("-");
    UE_LOG(LogTemp,Display,TEXT("CONTEXT_REFLECTION name=%s found=%d kind=%s expected_kind=%s raw_flags=%016llx computed_flags=%016llx actual_flags=%016llx expected_flags=%016llx actual_offset=%d expected_offset=%d actual_dim=%d expected_dim=1 property_class=%s expected_property_class=%s meta_class=%s expected_meta_class=%s"),
        Name,P!=nullptr,*ActualKind,Kind,Raw,Computed,P ? uint64(P->PropertyFlags) : 0ULL,Raw|Computed,
        P ? P->GetOffset_ForInternal() : -1,Offset,P ? P->ArrayDim : -1,*ActualClass,*ExpectedClass,*ActualMeta,*ExpectedMeta);
}
int32 LiveCount(UClass* Class)
{
    int32 Count = 0;
    for (TObjectIterator<UObject> It; It; ++It)
        if (It->GetClass() == Class && !It->HasAnyFlags(RF_ClassDefaultObject)) ++Count;
    return Count;
}
void Run(UWorld* World)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("ContextRuntimeProbe")) || bRan) return;
    bRan = true;
    UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
    ULocalPlayer* LP = GI ? GI->GetFirstGamePlayer() : nullptr;
    APlayerController* PC = LP ? LP->GetPlayerController(World) : nullptr;
    Check(TEXT("preconditions"), World && GI && LP && PC && PC->Player == LP && PC->GetWorld() == World && GEngine && GEngine->GetWorldContextFromWorld(World));
    if (!World || !GI || !LP || !PC || PC->Player != LP || PC->GetWorld() != World || !GEngine || !GEngine->GetWorldContextFromWorld(World)) return;
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_CASE identity player=%llx gi=%llx world=%llx"), (uint64)LP, (uint64)GI, (uint64)World);
    UClass* Legacy = UFortReplayContext::StaticClass();
    UClass* Front = FindObject<UClass>(nullptr, TEXT("/Script/FortniteUI.FortFrontEndContext"));
    UClass* Global = FindObject<UClass>(nullptr, TEXT("/Script/FortniteUI.FortGlobalUIContext"));
    UClass* Director = FindObject<UClass>(nullptr, TEXT("/Script/FortniteUI.FortUIDirector"));
    UClass* WidgetClass = FindObject<UClass>(nullptr, TEXT("/Script/CommonUI.CommonUserWidget"));
    Check(TEXT("classes"), Front && Global && Director && Legacy && WidgetClass && WidgetClass->IsChildOf(UUserWidget::StaticClass()) && !WidgetClass->HasAnyClassFlags(CLASS_Abstract) && !Legacy->HasAnyClassFlags(CLASS_Abstract));
    if (!Front || !Global || !Director || !Legacy || !WidgetClass || !WidgetClass->IsChildOf(UUserWidget::StaticClass()) || WidgetClass->HasAnyClassFlags(CLASS_Abstract) || Legacy->HasAnyClassFlags(CLASS_Abstract)) return;
    Check(TEXT("families"), Front->IsChildOf(ULocalPlayerSubsystem::StaticClass()) && Global->IsChildOf(ULocalPlayerSubsystem::StaticClass()) && Director->IsChildOf(UGameInstanceSubsystem::StaticClass()));
    auto Lookup = [](UObject* O, UClass* C) { return UBlueprintContextLibrary::GetContext(O, C); };
    UFunction* Function = UBlueprintContextLibrary::StaticClass()->FindFunctionByName(TEXT("GetContext"));
    FObjectProperty* Ret = Function ? FindFProperty<FObjectProperty>(Function, TEXT("ReturnValue")) : nullptr;
    FClassProperty* ClassProp = Function ? FindFProperty<FClassProperty>(Function, TEXT("Class")) : nullptr;
    FObjectProperty* Obj = Function ? FindFProperty<FObjectProperty>(Function, TEXT("ContextObject")) : nullptr;
    const uint64 ComputedFlags=FContextProbeObjectFlags::Get();
    Check(TEXT("reflection_computed_flags"), ComputedFlags==0x0008001040000200ULL);
    LogContextReflectionProperty(Function,TEXT("ReturnValue"),0x10000000000580ULL,ComputedFlags,16,TEXT("ObjectProperty"),USubsystem::StaticClass(),nullptr);
    LogContextReflectionProperty(Function,TEXT("Class"),0x14000000000080ULL,ComputedFlags,8,TEXT("ClassProperty"),UClass::StaticClass(),USubsystem::StaticClass());
    LogContextReflectionProperty(Function,TEXT("ContextObject"),0x10000000000080ULL,ComputedFlags,0,TEXT("ObjectProperty"),UObject::StaticClass(),nullptr);
    Check(TEXT("reflection_function"), Function && Function->GetOuter() == UBlueprintContextLibrary::StaticClass() && Function->FunctionFlags == 0x14022409 && Function->ParmsSize == 24 && Function->NumParms == 3 && Function->GetNativeFunc() != nullptr);
    Check(TEXT("reflection_return"), Ret && Ret->GetOffset_ForInternal() == 16 && Ret->ArrayDim == 1 && uint64(Ret->PropertyFlags) == (0x10000000000580ULL | ComputedFlags) && Ret->PropertyClass == USubsystem::StaticClass());
    Check(TEXT("reflection_class"), ClassProp && ClassProp->GetOffset_ForInternal() == 8 && ClassProp->ArrayDim == 1 && uint64(ClassProp->PropertyFlags) == (0x14000000000080ULL | ComputedFlags) && ClassProp->PropertyClass == UClass::StaticClass() && ClassProp->MetaClass == USubsystem::StaticClass());
    Check(TEXT("reflection_object"), Obj && Obj->GetOffset_ForInternal() == 0 && Obj->ArrayDim == 1 && uint64(Obj->PropertyFlags) == (0x10000000000080ULL | ComputedFlags) && Obj->PropertyClass == UObject::StaticClass());
    Check(TEXT("null"), Lookup(nullptr, Legacy) == nullptr);
    const int32 Before = LiveCount(Legacy);
    Check(TEXT("lookup_no_create"), Lookup(LP, Legacy) == nullptr && LiveCount(Legacy) == Before);
    UUserWidget* Widget = NewObject<UUserWidget>(LP, WidgetClass);
    Widget->SetPlayerContext(FLocalPlayerContext(PC));
    Check(TEXT("widget_owner"), Widget->GetOwningLocalPlayer() == LP);
    USubsystem* ExpectedFront = LP->GetSubsystemBase(TSubclassOf<ULocalPlayerSubsystem>(Front));
    USubsystem* ExpectedGlobal = LP->GetSubsystemBase(TSubclassOf<ULocalPlayerSubsystem>(Global));
    USubsystem* ExpectedDirector = GI->GetSubsystemBase(TSubclassOf<UGameInstanceSubsystem>(Director));
    Check(TEXT("subsystems_present"), ExpectedFront && ExpectedGlobal && ExpectedDirector);
    Check(TEXT("local_front"), ExpectedFront && Lookup(LP, Front) == ExpectedFront);
    Check(TEXT("local_global"), ExpectedGlobal && Lookup(LP, Global) == ExpectedGlobal);
    Check(TEXT("game_director"), ExpectedDirector && Lookup(LP, Director) == ExpectedDirector);
    Check(TEXT("player_selection"), ExpectedFront && Lookup(PC, Front) == ExpectedFront && Lookup(Widget, Front) == ExpectedFront && Lookup(World, Front) == ExpectedFront);
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_CASE create1_begin"));
    BlueprintContextRuntime::CreateContexts(LP); // diagnostic scope only, not an authentic startup hook
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_CASE create1_end"));
    UBlueprintContextBase* First = Cast<UBlueprintContextBase>(Lookup(LP, Legacy));
    Check(TEXT("registry_creation"), First && First->GetClass() == Legacy && First->GetOuter() == LP && Lookup(LP, UBlueprintContextBase::StaticClass()) == nullptr);
    TWeakObjectPtr<UBlueprintContextBase> FirstWeak(First);
    First = nullptr;
    CollectGarbage(RF_NoFlags);
    Check(TEXT("registry_gc_retains"), FirstWeak.IsValid() && Lookup(LP, Legacy) == FirstWeak.Get());
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_CASE create2_begin"));
    BlueprintContextRuntime::CreateContexts(LP);
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_CASE create2_end"));
    UBlueprintContextBase* Second = Cast<UBlueprintContextBase>(Lookup(LP, Legacy));
    Check(TEXT("registry_replacement"), FirstWeak.IsValid() && Second && Second != FirstWeak.Get() && Second->GetClass() == Legacy && Second->GetOuter() == LP);
    TWeakObjectPtr<UBlueprintContextBase> SecondWeak(Second);
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_CASE remove_begin"));
    BlueprintContextRuntime::DestroyContexts(LP);
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_CASE remove_end"));
    Check(TEXT("registry_remove"), !Lookup(LP, Legacy) && Second && Second->IsPendingKill());
    Check(TEXT("registry_pending_reachable"), SecondWeak.Get(true) == Second && Second && Second->IsPendingKill());
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_CASE gc_begin object=%llx"), (uint64)Second);
    Second = nullptr;
    CollectGarbage(RF_NoFlags);
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_CASE gc_end"));
    Check(TEXT("registry_collect_after_remove"), SecondWeak.Get(true) == nullptr);
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_PROBE_V1 END"));
}
FAutoConsoleCommandWithWorld Command(TEXT("Fort.ContextRuntimeProbe"), TEXT("Disposable process context diagnostic; requires -ContextRuntimeProbe"), FConsoleCommandWithWorldDelegate::CreateStatic(&Run));
}

namespace {
TWeakObjectPtr<ULocalPlayer> LifecyclePlayer;
void LifecycleBegin(UWorld* W)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("ContextRuntimeProbe")) || LifecyclePlayer.IsValid()) return;
    ULocalPlayer* P = W && W->GetGameInstance() ? W->GetGameInstance()->GetFirstGamePlayer() : nullptr;
    if (!P) { UE_LOG(LogTemp, Display, TEXT("CONTEXT_LIFE begin_fail")); return; }
    LifecyclePlayer = P;
    BlueprintContextRuntime::ArmLifecycleObservation();
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_LIFE begin player=%llx world=%llx gi=%llx"), (uint64)P, (uint64)W, (uint64)W->GetGameInstance());
    BlueprintContextRuntime::CreateContexts(P);
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_LIFE created player=%llx"), (uint64)P);
}
void LifecycleEnd(UWorld* W)
{
    if (!FParse::Param(FCommandLine::Get(), TEXT("ContextRuntimeProbe"))) return;
    ULocalPlayer* P = LifecyclePlayer.Get();
    if (!P || !W || !W->GetGameInstance() || W->GetGameInstance()->GetFirstGamePlayer() != P) { UE_LOG(LogTemp, Display, TEXT("CONTEXT_LIFE end_fail")); return; }
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_LIFE end player=%llx world=%llx"), (uint64)P, (uint64)W);
    BlueprintContextRuntime::DestroyContexts(P);
    UE_LOG(LogTemp, Display, TEXT("CONTEXT_LIFE removed player=%llx"), (uint64)P);
    LifecyclePlayer.Reset();
}
FAutoConsoleCommandWithWorld GFortContextLifecycleBeginCommand(TEXT("Fort.ContextLifecycleBegin"), TEXT("Diagnostic: keep contexts through natural world transition"), FConsoleCommandWithWorldDelegate::CreateStatic(&LifecycleBegin));
FAutoConsoleCommandWithWorld GFortContextLifecycleEndCommand(TEXT("Fort.ContextLifecycleEnd"), TEXT("Diagnostic: remove contexts after natural world transition"), FConsoleCommandWithWorldDelegate::CreateStatic(&LifecycleEnd));
}

namespace {
struct FContextProbeDriver
{
    // Called from FortniteGame::StartupModule after the command line exists.
    static void Register()
    {
        if (FParse::Param(FCommandLine::Get(), TEXT("ContextRuntimeProbe")))
            FCoreDelegates::OnEndFrame.AddStatic(&FContextProbeDriver::Tick);
    }
    static void Tick()
    {
        static const double StartSeconds = FPlatformTime::Seconds();
        static bool Started = false;
        static bool Finished = false;
        if (Finished) return;
        if (FPlatformTime::Seconds() - StartSeconds > 180.0)
        {
            Finished = true;
            UE_LOG(LogTemp, Error, TEXT("CONTEXT_DRIVER timeout"));
            FPlatformMisc::RequestExitWithStatus(false, 1);
            return;
        }
        if (!GEngine) return;
        FString Case;
        if (!FParse::Value(FCommandLine::Get(), TEXT("ContextProbeCase="), Case) || (Case != TEXT("registry") && Case != TEXT("lifecycle")))
        {
            Finished = true; UE_LOG(LogTemp, Error, TEXT("CONTEXT_DRIVER bad_case")); FPlatformMisc::RequestExitWithStatus(false, 1); return;
        }
        if (!Started)
        {
            for (const FWorldContext& WC : GEngine->GetWorldContexts())
            {
                UWorld* W = WC.World(); UGameInstance* GI = W ? W->GetGameInstance() : nullptr;
                ULocalPlayer* P = GI ? GI->GetFirstGamePlayer() : nullptr;
                APlayerController* PC = P ? P->GetPlayerController(W) : nullptr;
                if (!W || !W->IsGameWorld() || !P || !PC || PC->Player != P || PC->GetWorld() != W) continue;
                Started = true;
                UE_LOG(LogTemp, Display, TEXT("CONTEXT_DRIVER start case=%s world=%llx"), *Case, (uint64)W);
                if (Case == TEXT("registry"))
                {
                    Run(W); Finished = true; UE_LOG(LogTemp, Display, TEXT("CONTEXT_DRIVER complete case=registry")); FPlatformMisc::RequestExitWithStatus(false, 0);
                }
                else
                {
                    FString Map = TEXT("/Engine/Maps/Entry");
                    FParse::Value(FCommandLine::Get(), TEXT("ContextProbeTravelMap="), Map);
                    if (Map.IsEmpty()) { Finished = true; FPlatformMisc::RequestExitWithStatus(false, 1); return; }
                    LifecycleBegin(W);
                    UE_LOG(LogTemp, Display, TEXT("CONTEXT_DRIVER travel map=%s"), *Map);
                    UGameplayStatics::OpenLevel(W, FName(*Map), true);
                }
                return;
            }
        }
        else if (Case == TEXT("lifecycle") && BlueprintContextRuntime::LifecycleCallbacksObserved())
        {
            ULocalPlayer* P = LifecyclePlayer.Get(); UWorld* W = P ? P->GetWorld() : nullptr;
            if (W && W->GetGameInstance() && W->GetGameInstance()->GetFirstGamePlayer() == P)
            {
                LifecycleEnd(W); Finished = true; UE_LOG(LogTemp, Display, TEXT("CONTEXT_DRIVER complete case=lifecycle")); FPlatformMisc::RequestExitWithStatus(false, 0);
            }
        }
    }
};
}

void RegisterContextRuntimeProbeDriver()
{
    FContextProbeDriver::Register();
}

