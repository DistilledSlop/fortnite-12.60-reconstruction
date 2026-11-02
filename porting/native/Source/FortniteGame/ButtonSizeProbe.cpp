#include "ButtonInternalProbe.h"
#include "ActivationBaseProbe.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SizeBox.h"
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

namespace ButtonSizeDiagnostic
{
int32 Sequence=0; bool Failed=false, Registered=false; double Deadline=0; FDelegateHandle FrameHandle;
void Record(const TCHAR* Event,bool OK) { Failed |= !OK; UE_LOG(LogTemp,Display,TEXT("BTNSIZE v=1 seq=%d event=%s ok=%d"),++Sequence,Event,int32(OK)); }
void Run(UWorld* World,APlayerController* PC)
{
    Record(TEXT("begin"),true);
    TStrongObjectPtr<UActivationBaseProbeWidget> Owner(CreateWidget<UActivationBaseProbeWidget>(PC,UActivationBaseProbeWidget::StaticClass()));
    if (!Owner.IsValid() || !Owner->WidgetTree) { Record(TEXT("context"),false); return; }
    TStrongObjectPtr<UButtonInternalProbeWidget> Widget(NewObject<UButtonInternalProbeWidget>(Owner->WidgetTree));
    UButtonInternalProbeWidget* W=Widget.Get();
    USizeBox* Content=NewObject<USizeBox>(Owner->WidgetTree);
    Content->SetWidthOverride(200); Content->SetHeightOverride(120); W->AddChild(Content);
    Record(TEXT("context"), W->GetWorld()==World && Owner->GetWorld()==World
        && !W->FindFunction(TEXT("NativeSetMinimumWidth")) && !W->FindFunction(TEXT("NativeSetMinimumHeight")));
    W->NativeSetMinimumWidth(500);
    W->NativeSetMinimumHeight(600);
    Record(TEXT("size_prebuild_fields"), W->MinWidth == 500 && W->MinHeight == 600 && !W->Box().IsValid());
    TSharedPtr<SWidget> SizeBox = W->TakeWidget();
    SizeBox->SlatePrepass(1.0f);
    FVector2D Measured = SizeBox->GetDesiredSize();
    Record(TEXT("size_initial"), Measured.X >= 500.0f && Measured.Y >= 600.0f);
    W->NativeSetMinimumWidth(0);
    W->NativeSetMinimumHeight(0);
    SizeBox->SlatePrepass(1.0f);
    const FVector2D Baseline = SizeBox->GetDesiredSize();
    UE_LOG(LogTemp,Display,TEXT("BUTTON_SIZE_BASELINE width=%g height=%g"),Baseline.X,Baseline.Y);
    const int32 Wide = FMath::CeilToInt(Baseline.X) + 1000;
    const int32 Tall = FMath::CeilToInt(Baseline.Y) + 1000;
    W->NativeSetMinimumWidth(Wide);
    SizeBox->SlatePrepass(1.0f);
    Measured = SizeBox->GetDesiredSize();
    Record(TEXT("size_width_only"), W->MinWidth == Wide && W->MinHeight == 0 && Measured.X == float(Wide) && Measured.Y == Baseline.Y);
    W->NativeSetMinimumHeight(Tall);
    SizeBox->SlatePrepass(1.0f);
    Measured = SizeBox->GetDesiredSize();
    Record(TEXT("size_both"), W->MinHeight == Tall && Measured.X == float(Wide) && Measured.Y == float(Tall));
    W->NativeSetMinimumWidth(-10);
    W->NativeSetMinimumHeight(-20);
    SizeBox->SlatePrepass(1.0f);
    Measured = SizeBox->GetDesiredSize();
    Record(TEXT("size_minimum_not_override"), W->MinWidth == -10 && W->MinHeight == -20 && Measured == Baseline && Baseline.X >= 200 && Baseline.Y >= 120);
    W->NativeSetMinimumWidth(16777217);
    W->NativeSetMinimumHeight(Tall);
    SizeBox->SlatePrepass(1.0f);
    Measured = SizeBox->GetDesiredSize();
    Record(TEXT("size_int_float"), W->MinWidth == 16777217 && Measured.X == static_cast<float>(16777217));
    TWeakPtr<SWidget> OldBox=SizeBox;
    W->ReleaseProbe();
    SizeBox.Reset(); // The UWidget cache is weak; drop all strong Slate references.
    Record(TEXT("size_release"), !W->Box().IsValid() && OldBox.IsValid()==false);
    W->NativeSetMinimumWidth(Wide);
    W->NativeSetMinimumHeight(Tall);
    SizeBox = W->TakeWidget();
    SizeBox->SlatePrepass(1.0f);
    Measured = SizeBox->GetDesiredSize();
    Record(TEXT("size_rebuild_retains_fields"), W->Box().IsValid() && Measured.X == float(Wide) && Measured.Y == float(Tall));
}
void Finish() { FCoreDelegates::OnEndFrame.Remove(FrameHandle); Record(TEXT("complete"),!Failed); FPlatformMisc::RequestExitWithStatus(false,Failed?1:0); }
void Tick()
{
    if (FPlatformTime::Seconds()>Deadline) { Record(TEXT("timeout"),false); Finish(); return; }
    if (!GEngine) return;
    for (const FWorldContext& Context:GEngine->GetWorldContexts()) {
        UWorld* World=Context.World(); UGameInstance* GI=World?World->GetGameInstance():nullptr;
        ULocalPlayer* Player=GI?GI->GetFirstGamePlayer():nullptr;
        APlayerController* PC=Player?Player->GetPlayerController(World):nullptr;
        if (!World || !World->IsGameWorld() || !GI || !Player || !PC || PC->Player!=Player
            || PC->GetWorld()!=World || Player->GetGameInstance()!=GI || GI->GetWorld()!=World) continue;
        Run(World,PC); Finish(); return;
    }
}
}
void RegisterButtonSizeProbeDriver()
{
    using namespace ButtonSizeDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(),TEXT("ButtonSizeProbe"))) return;
    Registered=true; Deadline=FPlatformTime::Seconds()+180; FrameHandle=FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
