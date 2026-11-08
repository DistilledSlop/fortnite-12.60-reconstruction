#include "BorderDelegateFlowProbe.h"
#include "FortniteMapClasses.h"
#include "UFortKismetLibrary.h"
#include "UCommonUserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SViewport.h"

namespace BorderDelegateFlowDiagnostic
{
int32 Sequence = 0;
bool Failed = false, Registered = false;
double Deadline = 0;
FDelegateHandle FrameHandle;
void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("BORDERDELEGATEFLOW v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
}
bool Near(float A, float B) { return FMath::IsNearlyEqual(A, B, 0.01f); }
void Run(APlayerController* PC)
{
    Record(TEXT("begin"), true);
    TStrongObjectPtr<UCommonUserWidget> Owner(CreateWidget<UCommonUserWidget>(PC, UCommonUserWidget::StaticClass()));
    TStrongObjectPtr<UBorderFlowWidget> Widget(Owner.IsValid() && Owner->WidgetTree
        ? Owner->WidgetTree->ConstructWidget<UBorderFlowWidget>() : nullptr);
    UBorderFlowStyle* Style = GetMutableDefault<UBorderFlowStyle>();
    Record(TEXT("context"), Owner.IsValid() && Widget.IsValid() && Style && Widget->GetWorld() == PC->GetWorld()
        && Widget->Background.DrawAs == ESlateBrushDrawType::NoDrawType
        && Widget->bShowEffectWhenDisabled == GetDefault<UBorder>()->bShowEffectWhenDisabled
        && Widget->GetVisibility() == ESlateVisibility::SelfHitTestInvisible);
    if (!Widget.IsValid() || !Style) return;
    const FSlateBrush Saved = Style->Background;
    Style->Background.ImageSize = FVector2D(37, 19);
    Widget->Background.ImageSize = FVector2D(2, 3);
    Widget->Padding = FMargin(10, 20, 30, 40);
    Widget->MinimumPadding = FMargin(15, 5, 35, 10);
    Widget->bReducePaddingBySafezone = false;
    Widget->SetStyle(UBorderFlowStyle::StaticClass());
    Record(TEXT("before_slate"), Widget->Style.Get() == UBorderFlowStyle::StaticClass() && Widget->Background.ImageSize == FVector2D(2, 3));
    TSharedRef<SWidget> Slate = Widget->TakeWidget();
    TSharedPtr<SBorder> Border = Widget->Border();
    Record(TEXT("slate_style"), Border.IsValid() && Widget->Background.ImageSize == FVector2D(37, 19));
    if (!Border.IsValid()) { Style->Background = Saved; return; }
    Border->SlatePrepass();
    FVector2D Size = Border->GetDesiredSize();
    Record(TEXT("parent_padding"), Near(Size.X, 40) && Near(Size.Y, 60));
    int32 Before = Widget->Syncs;
    Style->Background.ImageSize = FVector2D(41, 23);
    Widget->SetStyle(UBorderFlowStyle::StaticClass());
    Record(TEXT("same_style"), Widget->Syncs == Before + 1 && Widget->Background.ImageSize == FVector2D(41, 23));
    Before = Widget->Syncs;
    Widget->SetStyle(nullptr);
    Record(TEXT("null_style"), Widget->Syncs == Before + 1 && !Widget->Style.Get() && Widget->Background.ImageSize == FVector2D(41, 23));
    Before = Widget->Syncs;
    Widget->SetStyle(TSubclassOf<UCommonBorderStyle>(UObject::StaticClass()));
    FClassProperty* StyleProperty = FindFProperty<FClassProperty>(Widget->GetClass(), TEXT("Style"));
    Record(TEXT("incompatible_style"), Widget->Syncs == Before + 1 && !Widget->Style.Get()
        && StyleProperty && StyleProperty->GetObjectPropertyValue_InContainer(Widget.Get()) == UObject::StaticClass()
        && Widget->Background.ImageSize == FVector2D(41, 23));
    FMargin Safe(0);
    TSharedPtr<SViewport> Viewport = FSlateApplication::Get().GetGameViewport();
    TSharedPtr<ISlateViewport> Interface = Viewport.IsValid() ? Viewport->GetViewportInterface().Pin() : nullptr;
    if (Interface.IsValid()) FSlateApplication::Get().GetSafeZoneSize(Safe, FVector2D(Interface->GetSize()));
    UE_LOG(LogTemp, Display, TEXT("BORDERDELEGATE_CONTEXT viewport_interface=%d"), int32(Interface.IsValid()));
    Widget->bReducePaddingBySafezone = true;
    Widget->SetStyle(UBorderFlowStyle::StaticClass());
    Border->SlatePrepass(); Size = Border->GetDesiredSize();
    const float Width = FMath::Max(15.f, 10.f - Safe.Left) + FMath::Max(35.f, 30.f - Safe.Right);
    const float Height = FMath::Max(5.f, 20.f - Safe.Top) + FMath::Max(10.f, 40.f - Safe.Bottom);
    Record(TEXT("safe_padding"), Near(Size.X, Width) && Near(Size.Y, Height) && Widget->Padding == FMargin(10, 20, 30, 40));
    Style->Background = Saved;
    UFunction* Delegate = FindObject<UFunction>(nullptr, TEXT("/Script/CommonUI.CommonActionCommited__DelegateSignature"));
    FProperty* Param = Delegate ? Delegate->PropertyLink : nullptr;
    const uint64 Required = CPF_Parm | CPF_OutParm;
    Record(TEXT("delegate_shape"), Delegate && Delegate->GetOuter() == FindObject<UPackage>(nullptr, TEXT("/Script/CommonUI"))
        && Delegate->HasAnyFunctionFlags(FUNC_Delegate) && !Delegate->HasAnyFunctionFlags(FUNC_MulticastDelegate)
        && Delegate->NumParms == 1 && Param && Param->GetFName() == TEXT("bPassThrough") && CastField<FBoolProperty>(Param)
        && Param->HasAllPropertyFlags(Required) && !Param->HasAnyPropertyFlags(CPF_ReferenceParm | CPF_ReturnParm));
    UFunction* Mobile = UFortKismetLibrary::StaticClass()->FindFunctionByName(TEXT("IsMobileGame"));
    FProperty* WorldParam = Mobile ? Mobile->FindPropertyByName(TEXT("WorldContextObject")) : nullptr;
    FProperty* ReturnParam = Mobile ? Mobile->GetReturnProperty() : nullptr;
    Record(TEXT("mobile_signature"), Mobile && Mobile->NumParms == 2 && WorldParam && CastField<FObjectPropertyBase>(WorldParam)
        && Mobile->HasAllFunctionFlags(FUNC_Static | FUNC_Native | FUNC_BlueprintCallable | FUNC_BlueprintPure)
        && WorldParam->HasAllPropertyFlags(CPF_Parm | CPF_ConstParm) && !WorldParam->HasAnyPropertyFlags(CPF_OutParm | CPF_ReturnParm)
        && ReturnParam && CastField<FBoolProperty>(ReturnParam)
        && ReturnParam->HasAllPropertyFlags(CPF_Parm | CPF_OutParm | CPF_ReturnParm)
        && !UFortKismetLibrary::IsMobileGame(PC->GetWorld()));
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
        if (!World || !World->IsGameWorld() || !Player || !PC || PC->Player != Player || PC->GetWorld() != World) continue;
        Run(PC); Finish(); return;
    }
}
}
void RegisterBorderDelegateFlowProbeDriver()
{
    using namespace BorderDelegateFlowDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("BorderDelegateFlowProbe"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
