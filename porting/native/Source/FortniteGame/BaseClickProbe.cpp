#include "BaseClickProbe.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "UCommonUISubsystem.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/GarbageCollection.h"

struct FCommonButtonBaseClickProbeAccess
{
    static void Click(UCommonButton* Button) { Button->NativeOnClicked(); }
    static FReply DoubleClick(UCommonButton* Button) { return Button->NativeHandleDoubleClicked(); }
    static void Select(UCommonButton* Button, bool Broadcast) { Button->NativeOnSelected(Broadcast); }
    static void Deselect(UCommonButton* Button, bool Broadcast) { Button->NativeOnDeselected(Broadcast); }
    static bool DefaultAnalytics(UCommonButton* Button, FString& A, FString& B, FString& C)
    { return Button->UCommonButton::NativeGetClickAnalytics(A, B, C); }
};
// Keep three distinct stack frames for debugger filtering of actual subsystem calls.
volatile int32 GBaseClickProbeBracketCompletions = 0;
FORCENOINLINE void BaseClickProbeNullWorld(UCommonButton* Button)
{ FCommonButtonBaseClickProbeAccess::Click(Button); ++GBaseClickProbeBracketCompletions; }
FORCENOINLINE void BaseClickProbeFalseAnalytics(UCommonButton* Button)
{ FCommonButtonBaseClickProbeAccess::Click(Button); ++GBaseClickProbeBracketCompletions; }
FORCENOINLINE void BaseClickProbeRealLookup(UCommonButton* Button)
{ FCommonButtonBaseClickProbeAccess::Click(Button); ++GBaseClickProbeBracketCompletions; }
UWorld* UBaseClickProbeButton::GetWorld() const { ++WorldCalls; return ProbeWorld.Get(); }
void UBaseClickProbeButton::ProcessEvent(UFunction* Function, void* Parameters)
{
    if (Function && Function->GetFName() == FName(TEXT("BP_OnClicked")))
    {
        Log.Add(TEXT("BP"));
        if (bReenter && !bInside)
        {
            bInside = true;
            FCommonButtonBaseClickProbeAccess::Click(this);
            bInside = false;
        }
    }
    if (Function && Function->GetFName() == FName(TEXT("BP_OnDoubleClicked")))
    {
        Log.Add(TEXT("DoubleBP"));
        if (bStopDouble)
        {
            if (UFunction* Setter = FindFunction(FName(TEXT("StopDoubleClickPropagation"))))
                ProcessEvent(Setter, nullptr);
        }
    }
    if (Function && Function->GetFName() == FName(TEXT("BP_OnSelected")))
    {
        Log.Add(TEXT("SelectedBP"));
        if (bReenterSelection && !bInside)
        {
            bInside = true;
            FCommonButtonBaseClickProbeAccess::Deselect(this, false);
            bInside = false;
        }
    }
    if (Function && Function->GetFName() == FName(TEXT("BP_OnDeselected")))
        Log.Add(TEXT("DeselectedBP"));
    Super::ProcessEvent(Function, Parameters);
}
void UBaseClickProbeButton::NativeOnCurrentTextStyleChanged() { Log.Add(TEXT("Text")); }
bool UBaseClickProbeButton::NativeGetClickAnalytics(FString& A, FString& B, FString& C)
{
    Log.Add(TEXT("Analytics"));
    A = TEXT("one"); B = TEXT("two"); C = TEXT("three");
    return bAnalytics;
}
void UBaseClickProbeReceiver::OnDynamic(UCommonButton* Clicked)
{
    if (Button) Button->Log.Add(Clicked == Button ? TEXT("Dynamic") : TEXT("DynamicWrongArgument"));
}
void UBaseClickProbeReceiver::OnDoubleDynamic(UCommonButton* Clicked)
{ if (Button) Button->Log.Add(Clicked == Button ? TEXT("DoubleDynamic") : TEXT("DoubleWrongArgument")); }
void UBaseClickProbeReceiver::OnSelectionDynamic(UCommonButton* SelectedButton, bool Selected)
{ if (Button) Button->Log.Add(SelectedButton != Button ? TEXT("SelectionWrongArgument") : Selected ? TEXT("SelectedDynamic") : TEXT("DeselectedDynamic")); }
void UBaseClickProbeReceiver::OnNative() { if (Button) Button->Log.Add(TEXT("Weak")); }

bool RunBaseClickProbe(UWorld* World, TFunctionRef<void(const TCHAR*, bool)> Record)
{
    bool All = true;
    auto Check = [&All, &Record](const TCHAR* Name, bool Pass) { All &= Pass; Record(Name, Pass); };
    TStrongObjectPtr<UBaseClickProbeReceiver> Outer(NewObject<UBaseClickProbeReceiver>());
    TStrongObjectPtr<UBaseClickProbeButton> B(NewObject<UBaseClickProbeButton>(Outer.Get(), FName(TEXT("ClickProbe_42"))));
    TStrongObjectPtr<UBaseClickProbeReceiver> R(NewObject<UBaseClickProbeReceiver>());
    R->Button = B.Get();
    B->OnButtonClicked.AddDynamic(R.Get(), &UBaseClickProbeReceiver::OnDynamic);
    FDelegateHandle First = B->NativeClicked.AddLambda([&] { B->Log.Add(TEXT("First")); });
    B->NativeClicked.AddLambda([&] {
        B->Log.Add(TEXT("Second")); B->NativeClicked.Remove(First);
        B->NativeClicked.AddLambda([&] { B->Log.Add(TEXT("Added")); });
    });
    FCommonButtonBaseClickProbeAccess::Click(B.Get());
    Check(TEXT("reverse_mutation_dynamic_order"), B->Log == TArray<FString>({TEXT("BP"), TEXT("Second"), TEXT("Dynamic"), TEXT("Analytics")}) && B->WorldCalls == 0);
    // Compaction uses RemoveAtSwap: Added replaces the removed First slot.
    B->Log.Reset(); FCommonButtonBaseClickProbeAccess::Click(B.Get());
    Check(TEXT("added_binding_next_broadcast"), B->Log == TArray<FString>({TEXT("BP"), TEXT("Second"), TEXT("Added"), TEXT("Dynamic"), TEXT("Analytics")}));
    B->NativeClicked.Clear(); B->OnButtonClicked.Clear(); B->Log.Reset(); B->bReenter = true;
    FCommonButtonBaseClickProbeAccess::Click(B.Get());
    Check(TEXT("bp_synchronous_reentry"), B->Log == TArray<FString>({TEXT("BP"), TEXT("BP"), TEXT("Analytics"), TEXT("Analytics")}));
    B->bReenter = false; B->bAnalytics = true; B->Log.Reset(); B->WorldCalls = 0;
    BaseClickProbeNullWorld(B.Get());
    Check(TEXT("true_analytics_no_world"), B->Log == TArray<FString>({TEXT("BP"), TEXT("Analytics")}) && B->WorldCalls == 1);
    B->bAnalytics = false; B->Log.Reset(); B->WorldCalls = 0;
    UBaseClickProbeReceiver* Dead = NewObject<UBaseClickProbeReceiver>();
    Dead->Button = B.Get();
    TWeakObjectPtr<UBaseClickProbeReceiver> WeakDead(Dead);
    B->NativeClicked.AddUObject(Dead, &UBaseClickProbeReceiver::OnNative);
    B->OnButtonClicked.AddDynamic(Dead, &UBaseClickProbeReceiver::OnDynamic);
    Dead = nullptr;
    CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
    const bool DeadBefore = !WeakDead.IsValid() && B->OnButtonClicked.IsBound();
    FCommonButtonBaseClickProbeAccess::Click(B.Get());
    Check(TEXT("weak_receivers_and_dynamic_compaction"), DeadBefore && !B->OnButtonClicked.IsBound() && !B->NativeClicked.IsBound() && B->Log == TArray<FString>({TEXT("BP"), TEXT("Analytics")}));
    FString A, C, D;
    Check(TEXT("default_analytics_name_suffix"), FCommonButtonBaseClickProbeAccess::DefaultAnalytics(B.Get(), A, C, D)
        && A == B->GetName() && A == TEXT("ClickProbe_42") && C == TEXT("None") && D == TEXT("None"));
    B->ProbeWorld = World; B->bAnalytics = false; B->WorldCalls = 0; B->Log.Reset();
    BaseClickProbeFalseAnalytics(B.Get());
    Check(TEXT("false_analytics_skips_world"), B->Log == TArray<FString>({TEXT("BP"), TEXT("Analytics")}) && B->WorldCalls == 0);
    const bool RealContext = World && World->IsGameWorld() && World->GetGameInstance();
    Check(TEXT("real_world_game_instance"), RealContext);
    B->bAnalytics = true; B->WorldCalls = 0; B->Log.Reset();
    // Break on FSubsystemCollectionBase::GetSubsystemInternal during this marked call.
    Record(TEXT("real_lookup_begin"), true);
    BaseClickProbeRealLookup(B.Get());
    Record(TEXT("real_lookup_end"), true);
    Check(TEXT("true_analytics_real_world"), RealContext && B->WorldCalls == 1 && B->Log == TArray<FString>({TEXT("BP"), TEXT("Analytics")}));
    // Invoke the reflected native setter first to verify each receiver clears stale state.
    UFunction* Stop = B->FindFunction(FName(TEXT("StopDoubleClickPropagation")));
    Check(TEXT("double_stop_setter_reflected"), Stop != nullptr);
    if (Stop) B->ProcessEvent(Stop, nullptr);
    B->OnButtonDoubleClicked.AddDynamic(R.Get(), &UBaseClickProbeReceiver::OnDoubleDynamic);
    B->NativeDoubleClicked.AddLambda([&] { B->Log.Add(TEXT("DoubleNative")); });
    B->Log.Reset();
    const FReply ClearReply = FCommonButtonBaseClickProbeAccess::DoubleClick(B.Get());
    Check(TEXT("double_clears_stale_stop_and_order"), Stop && !ClearReply.IsEventHandled() && B->Log == TArray<FString>({TEXT("DoubleBP"), TEXT("DoubleNative"), TEXT("DoubleDynamic")}));
    B->bStopDouble = true; B->Log.Reset();
    const FReply StopReply = FCommonButtonBaseClickProbeAccess::DoubleClick(B.Get());
    Check(TEXT("double_reflected_stop_and_order"), Stop && StopReply.IsEventHandled() && B->Log == TArray<FString>({TEXT("DoubleBP"), TEXT("DoubleNative"), TEXT("DoubleDynamic")}));
    B->bStopDouble = false;
    B->OnSelectedChanged.AddDynamic(R.Get(), &UBaseClickProbeReceiver::OnSelectionDynamic);
    B->NativeSelectedChanged.AddLambda([&](bool Selected) { B->Log.Add(Selected ? TEXT("SelectedNative") : TEXT("DeselectedNative")); });
    B->Log.Reset(); FCommonButtonBaseClickProbeAccess::Select(B.Get(), true);
    Check(TEXT("selection_broadcast_order"), B->Log == TArray<FString>({TEXT("SelectedBP"), TEXT("SelectedNative"), TEXT("SelectedDynamic"), TEXT("Text")}));
    B->Log.Reset(); FCommonButtonBaseClickProbeAccess::Deselect(B.Get(), true);
    Check(TEXT("deselection_broadcast_order"), B->Log == TArray<FString>({TEXT("DeselectedBP"), TEXT("DeselectedNative"), TEXT("DeselectedDynamic"), TEXT("Text")}));
    B->Log.Reset(); FCommonButtonBaseClickProbeAccess::Select(B.Get(), false);
    FCommonButtonBaseClickProbeAccess::Deselect(B.Get(), false);
    Check(TEXT("selection_suppressed_broadcast"), B->Log == TArray<FString>({TEXT("SelectedBP"), TEXT("Text"), TEXT("DeselectedBP"), TEXT("Text")}));
    B->bReenterSelection = true; B->Log.Reset();
    FCommonButtonBaseClickProbeAccess::Select(B.Get(), true);
    Check(TEXT("selection_synchronous_reentry"), B->Log == TArray<FString>({TEXT("SelectedBP"), TEXT("DeselectedBP"), TEXT("Text"), TEXT("SelectedNative"), TEXT("SelectedDynamic"), TEXT("Text")}));
    return All;
}
