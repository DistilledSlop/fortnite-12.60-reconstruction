#include "PanelOutroProbe.h"
#include "UCommonInputSubsystem.h"
#include "UCommonInputManager.h"
#include "UCommonGlobalInputHandler.h"
#include "UCommonUISubsystem.h"
#include "Engine/DataTable.h"
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
#include <limits>

#if !UE_BUILD_SHIPPING && !UE_BUILD_TEST
struct FPanelOutroProbeAccess {
    static bool Introed(UCommonActivatablePanel* W) { return W->bNativeIsIntroed; }
    static bool Bound(UCommonActivatablePanel* W, UCommonInputSubsystem* I) { return I->NativeInputMethodChanged.IsBoundToObject(W); }
    static bool HandleValid(UCommonActivatablePanel* W) { return W->NativeInputMethodHandle.IsValid(); }
    static void Bind(UCommonActivatablePanel* W, UPanelOutroProbeSink* S) { W->NativePreDeactivation.BindUObject(S,&UPanelOutroProbeSink::Pre); W->NativePostDeactivation.BindUObject(S,&UPanelOutroProbeSink::Post); }
    static void Cleanup(UCommonActivatablePanel* W,UCommonInputSubsystem* I) { I->NativeInputMethodChanged.Remove(W->NativeInputMethodHandle); W->NativeInputMethodHandle.Reset(); W->NativePreDeactivation.Unbind(); W->NativePostDeactivation.Unbind(); W->NativeActionRecords.Empty(); W->NativeInputManager.Reset(); }
    static void Manager(UCommonActivatablePanel* W,UCommonInputManager* M) { W->NativeInputManager=M; }
    static void Held(UCommonActivatablePanel* W,FDataTableRowHandle Key,float Duration) { W->NativeHeldAction=Key; W->NativeHeldActionDuration=Duration; }
    static bool IsHeld(UCommonActivatablePanel* W,FDataTableRowHandle Key,float Duration) { return W->NativeHeldAction==Key && W->NativeHeldActionDuration==Duration; }
    static void Add(UCommonActivatablePanel* W,FDataTableRowHandle Key,UPanelOutroProbeSink* S) { W->NativeActionRecords.FindOrAdd(Key).NativeProgress.AddUObject(S,&UPanelOutroProbeSink::Progress); }
    static void Clear(UCommonActivatablePanel* W) { W->NativeClearHeldAction(); }
    static bool SwapReady(UCommonActivatablePanel* W, UCommonInputManager* M) { return W->NativeInputManager.Get()==M && W->NativeHeldAction.DataTable && W->NativeHeldAction.RowName==FName(TEXT("ProbeRow")) && W->NativeHeldActionDuration==1.f; }
    static bool Cleared(UCommonActivatablePanel* W) { return W->NativeHeldAction==FDataTableRowHandle() && W->NativeHeldActionDuration==0 && W->NativeInputManager.IsValid() && !W->NativeInputManager->CurrentlyHeldActionInputHandler.GetObject() && !W->NativeInputManager->CurrentlyHeldActionInputHandler.GetInterface(); }
};
namespace PanelOutroProbeInternal {
int32 Seq=0; bool Failed=false,Registered=false; FDelegateHandle Frame; double Deadline=0;
void Record(const TCHAR* C,const TCHAR* E,UPanelOutroProbeWidget* W,bool OK=true) { Failed|=!OK; UE_LOG(LogTemp,Display,TEXT("PANELOUTRO v=1 seq=%d case=%s event=%s state=%d introed=%d ok=%d"),++Seq,C,E,W?int32(W->IsActivated()):-1,W?int32(FPanelOutroProbeAccess::Introed(W)):-1,int32(OK)); }
void Observe(UPanelOutroProbeWidget* W,const TCHAR* E,bool OK=true) { Record(*W->ProbeCase,E,W,OK); }
void Invoke(UPanelOutroProbeWidget* W,const TCHAR* Name) { UFunction* F=W->FindFunction(FName(Name)); if(!F || !F->GetNativeFunc()) { Observe(W,TEXT("missing_exec"),false); return; } W->ProcessEvent(F,nullptr); }
void SetInterface(UCommonInputManager* M,UPanelOutroProbeWidget* W) { M->CurrentlyHeldActionInputHandler.SetObject(W); M->CurrentlyHeldActionInputHandler.SetInterface(static_cast<ICommonActionHandlerInterface*>(W)); }
bool InterfaceIs(UCommonInputManager* M,UPanelOutroProbeWidget* W) { return M->CurrentlyHeldActionInputHandler.GetObject()==W && M->CurrentlyHeldActionInputHandler.GetInterface()==static_cast<ICommonActionHandlerInterface*>(W); }
bool InterfaceClear(UCommonInputManager* M) { return !M->CurrentlyHeldActionInputHandler.GetObject() && !M->CurrentlyHeldActionInputHandler.GetInterface(); }
void Run(ULocalPlayer* P,APlayerController* PC) {
    Record(TEXT("driver"),TEXT("begin"),nullptr);
    UGameInstance* GI=P->GetGameInstance(); auto* UI=GI->GetSubsystem<UCommonUISubsystem>(); auto* Input=P->GetSubsystem<UCommonInputSubsystem>();
    Record(TEXT("driver"),TEXT("subsystems"),nullptr,UI && Input); if(!UI || !Input) return;
    TStrongObjectPtr<UCommonInputManager> Old(UI->CommonInputManager), Manager(NewObject<UCommonInputManager>(UI)), Replacement(NewObject<UCommonInputManager>(UI));
    TStrongObjectPtr<UCommonGlobalInputHandler> Global(NewObject<UCommonGlobalInputHandler>(Manager.Get())); Manager->GlobalInputHandler=Global.Get(); UI->CommonInputManager=Manager.Get();
    TStrongObjectPtr<UDataTable> Table(NewObject<UDataTable>());
    FDataTableRowHandle Key; Key.DataTable=Table.Get(); Key.RowName=FName(TEXT("ProbeRow"));
    FDataTableRowHandle Missing=Key; Missing.RowName=FName(TEXT("MissingRow")); FDataTableRowHandle Empty;
    for(int Case=0;Case<2;++Case) {
        TStrongObjectPtr<UPanelOutroProbeWidget> W(CreateWidget<UPanelOutroProbeWidget>(PC,UPanelOutroProbeWidget::StaticClass()));
        if(!W.IsValid()) { Record(TEXT("driver"),TEXT("widget_failed"),nullptr,false); break; }
        W->ProbeCase=Case?TEXT("deferred"):TEXT("default");
        TStrongObjectPtr<UPanelOutroProbeSink> S(NewObject<UPanelOutroProbeSink>()); S->Widget=W.Get();
        Observe(W.Get(),TEXT("initial"),W->GetOwningLocalPlayer()==P && W->GetOwningPlayer()==PC && W->GetWorld()==PC->GetWorld() && !W->IsActivated() && !FPanelOutroProbeAccess::Introed(W.Get()));
        FPanelOutroProbeAccess::Manager(W.Get(),Manager.Get()); FPanelOutroProbeAccess::Bind(W.Get(),S.Get()); FPanelOutroProbeAccess::Add(W.Get(),Key,S.Get());
        FDelegateHandle N=W->NativeWidgetDeactivated.AddUObject(S.Get(),&UPanelOutroProbeSink::Native);
        W->BP_OnWidgetDeactivated.AddDynamic(S.Get(),&UPanelOutroProbeSink::BaseDynamic); W->OnWidgetDeactivated.AddDynamic(S.Get(),&UPanelOutroProbeSink::PanelDynamic);
        Invoke(W.Get(),TEXT("ActivateWidget")); Observe(W.Get(),TEXT("active"),W->IsActivated() && FPanelOutroProbeAccess::Introed(W.Get()) && FPanelOutroProbeAccess::Bound(W.Get(),Input));
        FPanelOutroProbeAccess::Held(W.Get(),Key,1); SetInterface(Manager.Get(),W.Get()); W->DeferOutro=Case!=0;
        Invoke(W.Get(),TEXT("DeactivateWidget"));
        if(Case) { Invoke(W.Get(),TEXT("DeactivateWidget")); Observe(W.Get(),TEXT("pending"),W->IsActivated() && FPanelOutroProbeAccess::Introed(W.Get()) && FPanelOutroProbeAccess::Bound(W.Get(),Input) && S->ProgressCount==1); Invoke(W.Get(),TEXT("EndOutro")); }
        Observe(W.Get(),TEXT("inactive"),!W->IsActivated() && !FPanelOutroProbeAccess::Introed(W.Get()) && !FPanelOutroProbeAccess::Bound(W.Get(),Input) && FPanelOutroProbeAccess::IsHeld(W.Get(),Empty,0) && InterfaceClear(Manager.Get()) && FPanelOutroProbeAccess::HandleValid(W.Get()));
        if(!Case) { Invoke(W.Get(),TEXT("ActivateWidget")); Observe(W.Get(),TEXT("reactivated"),W->IsActivated() && FPanelOutroProbeAccess::Introed(W.Get()) && FPanelOutroProbeAccess::Bound(W.Get(),Input)); Invoke(W.Get(),TEXT("DeactivateWidget")); Observe(W.Get(),TEXT("loop_inactive"),!W->IsActivated() && !FPanelOutroProbeAccess::Bound(W.Get(),Input)); }
        W->ProbeCase=TEXT("held");
        FPanelOutroProbeAccess::Manager(W.Get(),nullptr); FPanelOutroProbeAccess::Held(W.Get(),Key,2); FPanelOutroProbeAccess::Clear(W.Get()); Observe(W.Get(),TEXT("invalid_unchanged"),FPanelOutroProbeAccess::IsHeld(W.Get(),Key,2));
        FPanelOutroProbeAccess::Manager(W.Get(),Manager.Get());
        for(int I=0;I<5;++I) { FDataTableRowHandle K=I==0?Empty:I==3?Missing:Key; float D=I==1?-1.f:I==2?0.f:I==4?std::numeric_limits<float>::quiet_NaN():1.f; int Before=S->ProgressCount; FPanelOutroProbeAccess::Held(W.Get(),K,D); SetInterface(Manager.Get(),W.Get()); FPanelOutroProbeAccess::Clear(W.Get()); Observe(W.Get(),I==0?TEXT("empty"):I==1?TEXT("negative"):I==2?TEXT("zero"):I==3?TEXT("missing"):TEXT("nan"),S->ProgressCount==Before+(I==4?1:0) && FPanelOutroProbeAccess::IsHeld(W.Get(),Empty,0) && InterfaceClear(Manager.Get())); }
        SetInterface(Manager.Get(),W.Get()); SetInterface(Replacement.Get(),W.Get()); FPanelOutroProbeAccess::Manager(W.Get(),Manager.Get()); FPanelOutroProbeAccess::Held(W.Get(),Key,1); S->SwapManager=Replacement.Get(); S->PreviousManager=Manager.Get(); int Before=S->ProgressCount; FPanelOutroProbeAccess::Clear(W.Get()); Observe(W.Get(),TEXT("swap"),S->ProgressCount==Before+1 && InterfaceIs(Manager.Get(),W.Get()) && InterfaceClear(Replacement.Get()) && FPanelOutroProbeAccess::IsHeld(W.Get(),Empty,0)); S->SwapManager=nullptr; S->PreviousManager=nullptr;
        Manager->CurrentlyHeldActionInputHandler.SetObject(nullptr); Manager->CurrentlyHeldActionInputHandler.SetInterface(nullptr); Replacement->CurrentlyHeldActionInputHandler.SetObject(nullptr); Replacement->CurrentlyHeldActionInputHandler.SetInterface(nullptr);
        FPanelOutroProbeAccess::Cleanup(W.Get(),Input); W->NativeWidgetDeactivated.Remove(N); W->BP_OnWidgetDeactivated.RemoveDynamic(S.Get(),&UPanelOutroProbeSink::BaseDynamic); W->OnWidgetDeactivated.RemoveDynamic(S.Get(),&UPanelOutroProbeSink::PanelDynamic); S->Widget=nullptr;
        Observe(W.Get(),TEXT("cleanup"),!FPanelOutroProbeAccess::Bound(W.Get(),Input));
    }
    UI->CommonInputManager=Old.Get(); Record(TEXT("driver"),TEXT("restored"),nullptr,UI->CommonInputManager==Old.Get());
}
void Finish(bool OK) { FCoreDelegates::OnEndFrame.Remove(Frame); Record(TEXT("driver"),TEXT("complete"),nullptr,OK && !Failed); FPlatformMisc::RequestExitWithStatus(false,Failed?1:0); }
void Tick() { if(FPlatformTime::Seconds()>Deadline) { Record(TEXT("driver"),TEXT("timeout"),nullptr,false); Finish(false); return; } if(!GEngine)return; for(const FWorldContext& C:GEngine->GetWorldContexts()) { UWorld* W=C.World(); UGameInstance* GI=W?W->GetGameInstance():nullptr; ULocalPlayer* P=GI?GI->GetFirstGamePlayer():nullptr; APlayerController* PC=P?P->GetPlayerController(W):nullptr; if(!W || !W->IsGameWorld() || !GI || !P || !PC || PC->Player!=P || PC->GetWorld()!=W || P->GetGameInstance()!=GI || GI->GetWorld()!=W)continue; Run(P,PC); Finish(!Failed); return; } }
}
void UPanelOutroProbeWidget::ProcessEvent(UFunction* F,void* P) { const FName N=F?F->GetFName():NAME_None; if(N==FName(TEXT("OnBeginOutro"))) { PanelOutroProbeInternal::Observe(this,TEXT("begin_outro"),IsActivated() && FPanelOutroProbeAccess::Introed(this)); if(DeferOutro)return; } if(N==FName(TEXT("BP_OnDeactivated"))) PanelOutroProbeInternal::Observe(this,TEXT("base_bp"),!IsActivated() && !FPanelOutroProbeAccess::Introed(this)); Super::ProcessEvent(F,P); }
void UPanelOutroProbeWidget::NativeOutroComplete() { PanelOutroProbeInternal::Observe(this,TEXT("completion"),IsActivated() && !FPanelOutroProbeAccess::Introed(this)); }
void UPanelOutroProbeSink::Pre(UCommonActivatablePanel* P) { auto* I=Widget->GetOwningLocalPlayer()->GetSubsystem<UCommonInputSubsystem>(); PanelOutroProbeInternal::Observe(Widget,TEXT("pre"),P==Widget && !Widget->IsActivated() && !FPanelOutroProbeAccess::Bound(Widget,I) && FPanelOutroProbeAccess::Cleared(Widget)); }
void UPanelOutroProbeSink::Post(UCommonActivatablePanel* P) { PanelOutroProbeInternal::Observe(Widget,TEXT("post"),P==Widget && !Widget->IsActivated()); }
void UPanelOutroProbeSink::Native() { PanelOutroProbeInternal::Observe(Widget,TEXT("base_native"),!Widget->IsActivated()); }
void UPanelOutroProbeSink::BaseDynamic() { PanelOutroProbeInternal::Observe(Widget,TEXT("base_dynamic"),!Widget->IsActivated()); }
void UPanelOutroProbeSink::PanelDynamic(UCommonActivatablePanel* P) { PanelOutroProbeInternal::Observe(Widget,TEXT("panel_dynamic"),P==Widget && !Widget->IsActivated()); }
void UPanelOutroProbeSink::Progress(float V) { ++ProgressCount; PanelOutroProbeInternal::Observe(Widget,TEXT("progress"),V==0.f && (!SwapManager || (FPanelOutroProbeAccess::SwapReady(Widget,PreviousManager) && PanelOutroProbeInternal::InterfaceIs(PreviousManager,Widget)))); if(SwapManager) FPanelOutroProbeAccess::Manager(Widget,SwapManager); }
void RegisterPanelOutroProbeDriver() { if(PanelOutroProbeInternal::Registered || !FParse::Param(FCommandLine::Get(),TEXT("PanelOutroProbe")))return; PanelOutroProbeInternal::Registered=true; PanelOutroProbeInternal::Deadline=FPlatformTime::Seconds()+180; PanelOutroProbeInternal::Frame=FCoreDelegates::OnEndFrame.AddStatic(&PanelOutroProbeInternal::Tick); }
#else
void UPanelOutroProbeWidget::ProcessEvent(UFunction* F,void* P) { Super::ProcessEvent(F,P); }
void UPanelOutroProbeWidget::NativeOutroComplete() {}
void UPanelOutroProbeSink::Pre(UCommonActivatablePanel*) {} void UPanelOutroProbeSink::Post(UCommonActivatablePanel*) {} void UPanelOutroProbeSink::Native() {} void UPanelOutroProbeSink::Progress(float) {} void UPanelOutroProbeSink::BaseDynamic() {} void UPanelOutroProbeSink::PanelDynamic(UCommonActivatablePanel*) {}
void RegisterPanelOutroProbeDriver() {}
#endif
