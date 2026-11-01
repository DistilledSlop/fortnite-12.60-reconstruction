#include "ActivationBaseProbe.h"
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

namespace ActivationBaseProbeDiagnosticInternal {
int32 Sequence = 0;
bool Failed = false;
FDelegateHandle FrameHandle;
double Deadline = 0;
bool Registered = false;
void Record(const TCHAR* Case, const TCHAR* Event, UActivationBaseProbeWidget* W, bool OK = true)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("ACTBASE v=1 seq=%d case=%s event=%s state=%d visibility=%d ok=%d"),
        ++Sequence, Case, Event, W ? int32(W->IsActivated()) : -1,
        W ? int32(W->GetVisibility()) : -1, int32(OK));
}
void Observe(const TCHAR* Event, UActivationBaseProbeWidget* W)
{
    Record(W ? *W->ProbeCase : TEXT("invalid"), Event, W, W != nullptr);
}
bool Defaults(const UCommonActivatableWidget* W)
{
    return W && !W->bAutoActivate && !W->bIsBackHandler && W->bSupportsActivationFocus &&
        !W->bIsModal && !W->bAutoRestoreFocus && !W->bSetVisibilityOnActivated &&
        W->ActivatedVisibility == ESlateVisibility::SelfHitTestInvisible &&
        !W->bSetVisibilityOnDeactivated && W->DeactivatedVisibility == ESlateVisibility::Collapsed &&
        !W->bIsActive && !W->IsActivated() && W->GetVisibility() == ESlateVisibility::SelfHitTestInvisible;
}
bool Metadata(UActivationBaseProbeWidget* W, const TCHAR* Name, uint32 Flags, bool ReturnsBool)
{
    UFunction* F = W->FindFunction(FName(Name));
    FBoolProperty* Return = F ? CastField<FBoolProperty>(F->GetReturnProperty()) : nullptr;
    return F && F->GetOuterUClass() == UCommonActivatableWidget::StaticClass() &&
        uint32(F->FunctionFlags) == Flags && F->GetNativeFunc() != nullptr &&
        F->NumParms == (ReturnsBool ? 1 : 0) &&
        (ReturnsBool ? Return && Return->HasAnyPropertyFlags(CPF_ReturnParm) &&
            F->ParmsSize == sizeof(bool) : F->GetReturnProperty() == nullptr && F->ParmsSize == 0);
}
void Invoke(UActivationBaseProbeWidget* W, const TCHAR* Name)
{
    UFunction* F = W->FindFunction(FName(Name));
    if (!F || !F->HasAnyFunctionFlags(FUNC_Native) || !F->GetNativeFunc())
    { Record(*W->ProbeCase, TEXT("missing_exec"), W, false); return; }
    W->ProcessEvent(F, nullptr);
}
void State(UActivationBaseProbeWidget* W, const TCHAR* Event, bool Active, ESlateVisibility Visibility)
{
    struct { bool ReturnValue; } Result = { !Active };
    UFunction* F = W->FindFunction(FName(TEXT("IsActivated")));
    if (F && F->GetNativeFunc()) W->ProcessEvent(F, &Result);
    Record(*W->ProbeCase, Event, W, F && F->GetNativeFunc() && Result.ReturnValue == Active &&
        W->IsActivated() == Active && W->bIsActive == Active && W->GetVisibility() == Visibility);
}
void Run(ULocalPlayer* P, APlayerController* PC)
{
    Record(TEXT("driver"), TEXT("begin"), nullptr);
    TStrongObjectPtr<UActivationBaseProbeWidget> Widget(CreateWidget<UActivationBaseProbeWidget>(PC, UActivationBaseProbeWidget::StaticClass()));
    UActivationBaseProbeWidget* W = Widget.Get();
    Record(TEXT("defaults"), TEXT("ctor_instance"), W, Defaults(W) && W->GetOwningLocalPlayer() == P &&
        W->GetOwningPlayer() == PC && W->GetWorld() == PC->GetWorld());
    if (!W) return;
    auto* CDO = GetDefault<UActivationBaseProbeWidget>();
    Record(TEXT("defaults"), TEXT("ctor_cdo"), const_cast<UActivationBaseProbeWidget*>(CDO), Defaults(CDO));
    Record(TEXT("metadata"), TEXT("ActivateWidget"), W, Metadata(W, TEXT("ActivateWidget"), 0x04020401, false));
    Record(TEXT("metadata"), TEXT("DeactivateWidget"), W, Metadata(W, TEXT("DeactivateWidget"), 0x04020401, false));
    Record(TEXT("metadata"), TEXT("IsActivated"), W, Metadata(W, TEXT("IsActivated"), 0x54020401, true));
    for (const TCHAR* Name : { TEXT("BP_OnActivated"), TEXT("BP_OnDeactivated") })
    {
        UFunction* F = W->FindFunction(FName(Name));
        Record(TEXT("metadata"), Name, W, F && F->GetOuterUClass() == UCommonActivatableWidget::StaticClass() &&
            uint32(F->FunctionFlags) == 0x08080800 && F->NumParms == 0 && F->ParmsSize == 0 && !F->GetReturnProperty());
    }
    for (const TCHAR* Name : { TEXT("BP_OnWidgetActivated"), TEXT("BP_OnWidgetDeactivated") })
    {
        FMulticastDelegateProperty* F = FindFProperty<FMulticastDelegateProperty>(UCommonActivatableWidget::StaticClass(), FName(Name));
        Record(TEXT("metadata"), Name, W, F && F->SignatureFunction && F->SignatureFunction->NumParms == 0 &&
            F->SignatureFunction->ParmsSize == 0 && !F->SignatureFunction->GetReturnProperty());
    }
    TStrongObjectPtr<UActivationBaseProbeSink> Sink(NewObject<UActivationBaseProbeSink>());
    Sink->Widget = W;
    const FDelegateHandle A = W->NativeWidgetActivated.AddUObject(Sink.Get(), &UActivationBaseProbeSink::NativeActivated);
    const FDelegateHandle D = W->NativeWidgetDeactivated.AddUObject(Sink.Get(), &UActivationBaseProbeSink::NativeDeactivated);
    W->BP_OnWidgetActivated.AddDynamic(Sink.Get(), &UActivationBaseProbeSink::DynamicActivated);
    W->BP_OnWidgetDeactivated.AddDynamic(Sink.Get(), &UActivationBaseProbeSink::DynamicDeactivated);
    W->ProbeCase = TEXT("disabled");
    W->SetVisibility(ESlateVisibility::Hidden);
    State(W, TEXT("initial"), false, ESlateVisibility::Hidden);
    Invoke(W, TEXT("DeactivateWidget")); State(W, TEXT("repeat_inactive"), false, ESlateVisibility::Hidden);
    Invoke(W, TEXT("ActivateWidget")); State(W, TEXT("active"), true, ESlateVisibility::Hidden);
    Invoke(W, TEXT("ActivateWidget")); State(W, TEXT("repeat_active"), true, ESlateVisibility::Hidden);
    Invoke(W, TEXT("DeactivateWidget")); State(W, TEXT("inactive"), false, ESlateVisibility::Hidden);
    Invoke(W, TEXT("DeactivateWidget")); State(W, TEXT("repeat_inactive_after"), false, ESlateVisibility::Hidden);
    W->ProbeCase = TEXT("enabled");
    W->bSetVisibilityOnActivated = true; W->ActivatedVisibility = ESlateVisibility::HitTestInvisible;
    W->bSetVisibilityOnDeactivated = true; W->DeactivatedVisibility = ESlateVisibility::Collapsed;
    Invoke(W, TEXT("ActivateWidget")); State(W, TEXT("active"), true, ESlateVisibility::HitTestInvisible);
    Invoke(W, TEXT("DeactivateWidget")); State(W, TEXT("inactive"), false, ESlateVisibility::Collapsed);
    W->ProbeCase = TEXT("reentry"); W->ReenterOnActivation = true;
    Invoke(W, TEXT("ActivateWidget")); State(W, TEXT("inactive"), false, ESlateVisibility::Collapsed);
    Record(TEXT("reentry"), TEXT("consumed"), W, !W->ReenterOnActivation);
    W->NativeWidgetActivated.Remove(A); W->NativeWidgetDeactivated.Remove(D);
    W->BP_OnWidgetActivated.RemoveDynamic(Sink.Get(), &UActivationBaseProbeSink::DynamicActivated);
    W->BP_OnWidgetDeactivated.RemoveDynamic(Sink.Get(), &UActivationBaseProbeSink::DynamicDeactivated);
    Sink->Widget = nullptr;
    Record(TEXT("driver"), TEXT("cleanup"), W, !W->NativeWidgetActivated.IsBound() &&
        !W->NativeWidgetDeactivated.IsBound() && !W->BP_OnWidgetActivated.IsBound() && !W->BP_OnWidgetDeactivated.IsBound());
}
void Finish(bool OK)
{
    FCoreDelegates::OnEndFrame.Remove(FrameHandle);
    Record(TEXT("driver"), TEXT("complete"), nullptr, OK && !Failed);
    FPlatformMisc::RequestExitWithStatus(false, Failed ? 1 : 0);
}
void Tick()
{
    if (FPlatformTime::Seconds() > Deadline)
    { Record(TEXT("driver"), TEXT("timeout"), nullptr, false); Finish(false); return; }
    if (!GEngine) return;
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* W = Context.World(); UGameInstance* GI = W ? W->GetGameInstance() : nullptr;
        ULocalPlayer* P = GI ? GI->GetFirstGamePlayer() : nullptr;
        APlayerController* PC = P ? P->GetPlayerController(W) : nullptr;
        if (!W || !W->IsGameWorld() || !GI || !P || !PC || PC->Player != P ||
            PC->GetWorld() != W || P->GetGameInstance() != GI || GI->GetWorld() != W) continue;
        Run(P, PC); Finish(!Failed); return;
    }
}
}
void UActivationBaseProbeWidget::ProcessEvent(UFunction* Function, void* Parameters)
{
    const FName Name = Function ? Function->GetFName() : NAME_None;
    const bool Activate = Name == FName(TEXT("BP_OnActivated"));
    const bool Deactivate = Name == FName(TEXT("BP_OnDeactivated"));
    if (Activate || Deactivate) ActivationBaseProbeDiagnosticInternal::Observe(Activate ? TEXT("bp_activated") : TEXT("bp_deactivated"), this);
    Super::ProcessEvent(Function, Parameters);
    if (Activate && ReenterOnActivation)
    {
        ReenterOnActivation = false;
        ActivationBaseProbeDiagnosticInternal::Invoke(this, TEXT("DeactivateWidget"));
    }
}
void UActivationBaseProbeSink::NativeActivated() { ActivationBaseProbeDiagnosticInternal::Observe(TEXT("native_activated"), Widget); }
void UActivationBaseProbeSink::NativeDeactivated() { ActivationBaseProbeDiagnosticInternal::Observe(TEXT("native_deactivated"), Widget); }
void UActivationBaseProbeSink::DynamicActivated() { ActivationBaseProbeDiagnosticInternal::Observe(TEXT("dynamic_activated"), Widget); }
void UActivationBaseProbeSink::DynamicDeactivated() { ActivationBaseProbeDiagnosticInternal::Observe(TEXT("dynamic_deactivated"), Widget); }
void RegisterActivationBaseProbeDriver()
{
    if (ActivationBaseProbeDiagnosticInternal::Registered || !FParse::Param(FCommandLine::Get(), TEXT("ActivationBaseProbe"))) return;
    ActivationBaseProbeDiagnosticInternal::Registered = true; ActivationBaseProbeDiagnosticInternal::Deadline = FPlatformTime::Seconds() + 180.0;
    ActivationBaseProbeDiagnosticInternal::FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&ActivationBaseProbeDiagnosticInternal::Tick);
}
