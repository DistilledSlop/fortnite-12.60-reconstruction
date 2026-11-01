#include "PanelIntroProbe.h"
#include "UCommonInputSubsystem.h"
#include "UCommonInputManager.h"
#include "UCommonGlobalInputHandler.h"
#include "UCommonUISubsystem.h"
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

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
// Friend provides typed observation/binding only; no production state setter.
struct FPanelIntroProbeAccess {
    static void Bind(UCommonActivatablePanel* W, UPanelIntroProbeSink* S) {
        W->NativePreActivation.BindUObject(S, &UPanelIntroProbeSink::Pre);
        W->NativePostActivation.BindUObject(S, &UPanelIntroProbeSink::Post);
    }
    static void Unbind(UCommonActivatablePanel* W, UCommonInputSubsystem* Input) {
        Input->NativeInputMethodChanged.Remove(W->NativeInputMethodHandle);
        W->NativeInputMethodHandle.Reset();
        W->NativePreActivation.Unbind(); W->NativePostActivation.Unbind();
    }
    static bool Introed(UCommonActivatablePanel* W) { return W->bNativeIsIntroed; }
    static bool Initial(const UCommonActivatablePanel* W) { return !W->bNativeIsIntroed && !W->NativeInputManager.IsValid() && !W->NativeInputMethodHandle.IsValid() && !W->NativePreActivation.IsBound() && !W->NativePostActivation.IsBound(); }
    static bool Bound(UCommonActivatablePanel* W) { return W->NativeInputMethodHandle.IsValid(); }
};
namespace PanelIntroProbeInternal {
int32 Seq=0; bool Failed=false, Registered=false; FDelegateHandle Frame; double Deadline=0;
void Record(const TCHAR* C,const TCHAR* E, UPanelIntroProbeWidget* W,bool OK=true) {
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("PANELINTRO v=1 seq=%d case=%s event=%s state=%d visibility=%d ok=%d"), ++Seq,C,E,W?int32(W->IsActivated()):-1,W?int32(W->GetVisibility()):-1,int32(OK));
}
void Observe(UPanelIntroProbeWidget* W,const TCHAR* E,bool OK=true) { Record(*W->ProbeCase,E,W,OK); }
void Invoke(UPanelIntroProbeWidget* W,const TCHAR* Name) {
    UFunction* F=W->FindFunction(FName(Name));
    if(!F || !F->GetNativeFunc()) { Observe(W,TEXT("missing_exec"),false); return; }
    W->ProcessEvent(F,nullptr);
}
bool Metadata(UPanelIntroProbeWidget* W,const TCHAR* Name,uint32 Flags,int32 Count) {
    UFunction* F=W->FindFunction(FName(Name));
    return F && F->GetOuterUClass()==UCommonActivatablePanel::StaticClass() && uint32(F->FunctionFlags)==Flags && F->NumParms==Count && !F->GetReturnProperty();
}
void Run(ULocalPlayer* P,APlayerController* PC) {
    Record(TEXT("driver"),TEXT("begin"),nullptr);
    UGameInstance* GI=P->GetGameInstance(); auto* UI=GI->GetSubsystem<UCommonUISubsystem>(); auto* Input=P->GetSubsystem<UCommonInputSubsystem>();
    Record(TEXT("driver"),TEXT("subsystems"),nullptr,UI && Input);
    if(!UI || !Input) return;
    TStrongObjectPtr<UCommonInputManager> Old(UI->CommonInputManager);
    TStrongObjectPtr<UCommonInputManager> Manager(NewObject<UCommonInputManager>(UI));
    TStrongObjectPtr<UCommonGlobalInputHandler> Global(NewObject<UCommonGlobalInputHandler>(Manager.Get()));
    Manager->GlobalInputHandler=Global.Get(); UI->CommonInputManager=Manager.Get();
    for(int Case=0;Case<3;++Case) {
        TStrongObjectPtr<UPanelIntroProbeWidget> W(CreateWidget<UPanelIntroProbeWidget>(PC,UPanelIntroProbeWidget::StaticClass()));
        if(!W.IsValid()) { Record(TEXT("driver"),TEXT("widget_failed"),nullptr,false); break; }
        W->ProbeCase=Case==0?TEXT("default"):Case==1?TEXT("deferred"):TEXT("reentry");
        Observe(W.Get(),TEXT("initial"),W->GetOwningLocalPlayer()==P && W->GetOwningPlayer()==PC && W->GetWorld()==PC->GetWorld() && !W->IsActivated() && FPanelIntroProbeAccess::Initial(W.Get()));
        if(Case==0) {
            auto* CDO=GetDefault<UPanelIntroProbeWidget>();
            Observe(W.Get(),TEXT("defaults"),FPanelIntroProbeAccess::Initial(CDO) && !CDO->bIsActive && !CDO->bAutoActivate && !CDO->bSetVisibilityOnActivated);
            Observe(W.Get(),TEXT("metadata_begin"),Metadata(W.Get(),TEXT("OnBeginIntro"),0x08080c00,0));
            Observe(W.Get(),TEXT("metadata_end"),Metadata(W.Get(),TEXT("EndIntro"),0x04080401,0));
            Observe(W.Get(),TEXT("metadata_input"),Metadata(W.Get(),TEXT("OnInputModeChanged"),0x08080800,1));
        }
        TStrongObjectPtr<UPanelIntroProbeSink> S(NewObject<UPanelIntroProbeSink>()); S->Widget=W.Get();
        FPanelIntroProbeAccess::Bind(W.Get(),S.Get());
        FDelegateHandle N=W->NativeWidgetActivated.AddUObject(S.Get(),&UPanelIntroProbeSink::Native);
        W->BP_OnWidgetActivated.AddDynamic(S.Get(),&UPanelIntroProbeSink::BaseDynamic);
        W->OnWidgetActivated.AddDynamic(S.Get(),&UPanelIntroProbeSink::PanelDynamic);
        W->SetVisibility(ESlateVisibility::Hidden); W->bSetVisibilityOnActivated=true; W->ActivatedVisibility=ESlateVisibility::HitTestInvisible;
        Global->NativePriorityFilter=0;
        W->DeferIntro=Case==1; W->ReenterCompletion=Case==2;
        Invoke(W.Get(),TEXT("ActivateWidget"));
        if(Case==1) {
            Invoke(W.Get(),TEXT("ActivateWidget"));
            Observe(W.Get(),TEXT("pending"),FPanelIntroProbeAccess::Introed(W.Get()) && !W->IsActivated() && !FPanelIntroProbeAccess::Bound(W.Get()));
            Invoke(W.Get(),TEXT("EndIntro"));
        }
        Observe(W.Get(),TEXT("active"),W->IsActivated() && FPanelIntroProbeAccess::Introed(W.Get()) && FPanelIntroProbeAccess::Bound(W.Get()) && Global->NativePriorityFilter==0x12345678 && W->GetVisibility()==ESlateVisibility::HitTestInvisible);
        if(Case==0) {
            Input->NativeInputMethodChanged.Broadcast(ECommonInputType::Gamepad);
            Input->NativeInputMethodChanged.Broadcast(ECommonInputType::MouseAndKeyboard);
            Invoke(W.Get(),TEXT("EndIntro"));
            Observe(W.Get(),TEXT("already_active"),W->IsActivated());
        }
        FPanelIntroProbeAccess::Unbind(W.Get(),Input);
        W->NativeWidgetActivated.Remove(N); W->BP_OnWidgetActivated.RemoveDynamic(S.Get(),&UPanelIntroProbeSink::BaseDynamic); W->OnWidgetActivated.RemoveDynamic(S.Get(),&UPanelIntroProbeSink::PanelDynamic);
        S->Widget=nullptr;
        Observe(W.Get(),TEXT("cleanup"),!FPanelIntroProbeAccess::Bound(W.Get()));
    }
    UI->CommonInputManager=Old.Get();
    Record(TEXT("driver"),TEXT("restored"),nullptr,UI->CommonInputManager==Old.Get());
}
void Finish(bool OK) { FCoreDelegates::OnEndFrame.Remove(Frame); Record(TEXT("driver"),TEXT("complete"),nullptr,OK && !Failed); FPlatformMisc::RequestExitWithStatus(false,Failed?1:0); }
void Tick() {
    if(FPlatformTime::Seconds()>Deadline) { Record(TEXT("driver"),TEXT("timeout"),nullptr,false); Finish(false); return; }
    if(!GEngine) return;
    for(const FWorldContext& C:GEngine->GetWorldContexts()) { UWorld* W=C.World(); UGameInstance* GI=W?W->GetGameInstance():nullptr; ULocalPlayer* P=GI?GI->GetFirstGamePlayer():nullptr; APlayerController* PC=P?P->GetPlayerController(W):nullptr;
        if(!W || !W->IsGameWorld() || !GI || !P || !PC || PC->Player!=P || PC->GetWorld()!=W || P->GetGameInstance()!=GI || GI->GetWorld()!=W) continue;
        Run(P,PC); Finish(!Failed); return;
    }
}
}
void UPanelIntroProbeWidget::ProcessEvent(UFunction* F,void* P) {
    const FName N=F?F->GetFName():NAME_None;
    if(N==FName(TEXT("OnBeginIntro"))) { PanelIntroProbeInternal::Observe(this,TEXT("begin_intro")); if(DeferIntro) return; }
    if(N==FName(TEXT("BP_OnActivated"))) PanelIntroProbeInternal::Observe(this,TEXT("base_bp"));
    if(N==FName(TEXT("OnInputModeChanged"))) { bool B=P && *static_cast<bool*>(P); PanelIntroProbeInternal::Observe(this,B?TEXT("input_bp_gamepad"):TEXT("input_bp_other"),P!=nullptr); }
    Super::ProcessEvent(F,P);
}
void UPanelIntroProbeWidget::NativeIntroComplete() { PanelIntroProbeInternal::Observe(this,TEXT("completion")); if(ReenterCompletion) { ReenterCompletion=false; PanelIntroProbeInternal::Invoke(this,TEXT("EndIntro")); } }
void UPanelIntroProbeWidget::NativeInputMethodChangedHook(ECommonInputType T) { PanelIntroProbeInternal::Observe(this,T==ECommonInputType::Gamepad?TEXT("input_native_gamepad"):TEXT("input_native_other"),T==ECommonInputType::Gamepad || T==ECommonInputType::MouseAndKeyboard); }
void UPanelIntroProbeSink::Pre(UCommonActivatablePanel* P) { PanelIntroProbeInternal::Observe(Widget,TEXT("pre"),P==Widget); }
void UPanelIntroProbeSink::Post(UCommonActivatablePanel* P) { PanelIntroProbeInternal::Observe(Widget,TEXT("post"),P==Widget); }
void UPanelIntroProbeSink::Native() { PanelIntroProbeInternal::Observe(Widget,TEXT("base_native")); }
void UPanelIntroProbeSink::BaseDynamic() { PanelIntroProbeInternal::Observe(Widget,TEXT("base_dynamic")); }
void UPanelIntroProbeSink::PanelDynamic(UCommonActivatablePanel* P) { PanelIntroProbeInternal::Observe(Widget,TEXT("panel_dynamic"),P==Widget); }
void RegisterPanelIntroProbeDriver() { if(PanelIntroProbeInternal::Registered || !FParse::Param(FCommandLine::Get(),TEXT("PanelIntroProbe"))) return; PanelIntroProbeInternal::Registered=true; PanelIntroProbeInternal::Deadline=FPlatformTime::Seconds()+180; PanelIntroProbeInternal::Frame=FCoreDelegates::OnEndFrame.AddStatic(&PanelIntroProbeInternal::Tick); }
#else
void UPanelIntroProbeWidget::ProcessEvent(UFunction* F, void* P) { Super::ProcessEvent(F, P); }
void UPanelIntroProbeWidget::NativeIntroComplete() {}
void UPanelIntroProbeWidget::NativeInputMethodChangedHook(ECommonInputType) {}
void UPanelIntroProbeSink::Pre(UCommonActivatablePanel*) {}
void UPanelIntroProbeSink::Post(UCommonActivatablePanel*) {}
void UPanelIntroProbeSink::Native() {}
void UPanelIntroProbeSink::BaseDynamic() {}
void UPanelIntroProbeSink::PanelDynamic(UCommonActivatablePanel*) {}
void RegisterPanelIntroProbeDriver() {}
#endif
