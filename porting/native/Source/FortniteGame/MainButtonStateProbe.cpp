#include "MainButtonStateProbe.h"
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
#include "UObject/UnrealType.h"

namespace MainButtonStateDiagnosticInternal
{
int32 Sequence = 0;
bool Failed = false;
bool Registered = false;
double Deadline = 0;
FDelegateHandle FrameHandle;

void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("MAINBTN v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
}

void Run(APlayerController* PC, UWorld* World)
{
    Record(TEXT("begin"), true);
    TStrongObjectPtr<UMainButtonStateProbeWidget> Widget(CreateWidget<UMainButtonStateProbeWidget>(PC, UMainButtonStateProbeWidget::StaticClass()));
    UMainButtonStateProbeWidget* W = Widget.Get();
    const UMainButtonStateProbeWidget* CDO = GetDefault<UMainButtonStateProbeWidget>();
    Record(TEXT("defaults"), W && W->GetWorld() == World && CDO && W->NativeDefaults() && CDO->NativeDefaults() && W->IsInteractionEnabled());
    if (!W) return;
    UFunction* F = W->FindFunction(FName(TEXT("IsInteractionEnabled")));
    FBoolProperty* Return = F ? CastField<FBoolProperty>(F->GetReturnProperty()) : nullptr;
    const bool ValidFunction = F && F->GetOuterUClass() == UCommonButton::StaticClass() &&
        uint32(F->FunctionFlags) == 0x54020401 && F->GetNativeFunc() && F->NumParms == 1 &&
        F->ParmsSize == sizeof(bool) && Return && Return->IsNativeBool() && Return->HasAnyPropertyFlags(CPF_ReturnParm);
    Record(TEXT("metadata"), ValidFunction);
    for (int32 Visibility = 0; Visibility < 5; ++Visibility)
    {
        for (int32 Bits = 0; Bits < 8; ++Bits)
        {
            const bool WidgetEnabled = !!(Bits & 1);
            const bool NativeEnabled = !!(Bits & 2);
            const bool InteractionEnabled = !!(Bits & 4);
            W->Configure(ESlateVisibility(Visibility), WidgetEnabled, NativeEnabled, InteractionEnabled);
            const bool Expected = Visibility != int32(ESlateVisibility::Collapsed) &&
                Visibility != int32(ESlateVisibility::Hidden) && WidgetEnabled && NativeEnabled && InteractionEnabled;
            struct { bool ReturnValue; } Reflected = { !Expected };
            if (ValidFunction) W->ProcessEvent(F, &Reflected);
            const bool Direct = W->IsInteractionEnabled();
            const bool Alias = W->NativeIsInteractionEnabled();
            const bool OK = ValidFunction && Direct == Expected && Alias == Expected && Reflected.ReturnValue == Expected;
            Failed |= !OK;
            UE_LOG(LogTemp, Display, TEXT("MAINBTN v=1 seq=%d event=state visibility=%d bits=%d direct=%d alias=%d reflected=%d ok=%d"),
                ++Sequence, Visibility, Bits, int32(Direct), int32(Alias), int32(Reflected.ReturnValue), int32(OK));
        }
    }
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
        UWorld* W = Context.World();
        UGameInstance* GI = W ? W->GetGameInstance() : nullptr;
        ULocalPlayer* Player = GI ? GI->GetFirstGamePlayer() : nullptr;
        APlayerController* PC = Player ? Player->GetPlayerController(W) : nullptr;
        if (!W || !W->IsGameWorld() || !GI || !Player || !PC || PC->Player != Player ||
            PC->GetWorld() != W || Player->GetGameInstance() != GI || GI->GetWorld() != W) continue;
        Run(PC, W); Finish(); return;
    }
}
}

void RegisterMainButtonStateProbeDriver()
{
    using namespace MainButtonStateDiagnosticInternal;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("MainButtonStateProbe"))) return;
    Registered = true;
    Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
