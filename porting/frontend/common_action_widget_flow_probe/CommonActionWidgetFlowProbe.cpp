#include "CommonActionWidgetFlowProbe.h"
#include "CommonActionBindingData.h"
#include "CommonInputModuleRuntime.h"
#include "UCommonInputSettings.h"
#include "UCommonInputSubsystem.h"
#include "UCommonUserWidget.h"
#include "FCommonInputActionData.h"
#include "Engine/DataTable.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "UObject/StrongObjectPtr.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SOverlay.h"

namespace ActionWidgetFlowDiagnostic
{
int32 Sequence = 0;
bool Failed = false, Registered = false;
double Deadline = 0;
FDelegateHandle FrameHandle;
void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("ACTIONWIDGETFLOW v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
}
void Run(ULocalPlayer* Player, APlayerController* PC)
{
    Record(TEXT("begin"), true);
    UCommonInputSubsystem* Input = Player->GetSubsystem<UCommonInputSubsystem>();
    UCommonInputSettings* Settings = GetCommonInputSettings();
    IConsoleVariable* ShowKeys = IConsoleManager::Get().FindConsoleVariable(TEXT("CommonInput.ShowKeys"));
    TStrongObjectPtr<UCommonUserWidget> Owner(CreateWidget<UCommonUserWidget>(PC, UCommonUserWidget::StaticClass()));
    TStrongObjectPtr<UActionWidgetFlowWidget> Widget(Owner.IsValid() && Owner->WidgetTree
        ? Owner->WidgetTree->ConstructWidget<UActionWidgetFlowWidget>() : nullptr);
    Record(TEXT("context"), Input && Settings && ShowKeys && Widget.IsValid() && Owner.IsValid()
        && Widget->GetOwningLocalPlayer() == Player && Widget->GetWorld() == PC->GetWorld());
    if (!Input || !Settings || !ShowKeys || !Widget.IsValid() || !Owner.IsValid()) return;
    const auto SavedPlatform = Settings->CommonInputPlatformData[0];
    const auto SavedCurrent = Settings->CurrentPlatform;
    const auto SavedType = Input->CurrentInputType;
    const int32 SavedShow = ShowKeys->GetInt();
    UActionWidgetFlowController* Controller = GetMutableDefault<UActionWidgetFlowController>();
    const auto SavedControllerType = Controller->InputType;
    const auto SavedMap = Controller->InputBrushDataMap;
    Controller->InputType = ECommonInputType::MouseAndKeyboard;
    TStrongObjectPtr<UTexture2D> Texture(NewObject<UTexture2D>());
    FCommonInputKeyBrushConfiguration Entry;
    Entry.Key = EKeys::A; Entry.KeyBrush.SetResourceObject(Texture.Get()); Entry.KeyBrush.ImageSize = FVector2D(37, 19);
    Controller->InputBrushDataMap = {Entry};
    Settings->CommonInputPlatformData[0].ControllerData = {UActionWidgetFlowController::StaticClass()};
    Settings->CurrentPlatform.ControllerDataClasses.Empty();
    Input->CurrentInputType = ECommonInputType::MouseAndKeyboard;
    ShowKeys->SetWithCurrentPriority(1);
    TStrongObjectPtr<UDataTable> Table(NewObject<UDataTable>());
    Table->RowStruct = FCommonInputActionData::StaticStruct();
    FCommonInputActionData Row;
    Row.DisplayName = FText::FromString(TEXT("Tap action"));
    Row.HoldDisplayName = FText::FromString(TEXT("Hold action"));
    Row.KeyboardInputTypeInfo.Key = EKeys::A;
    Row.KeyboardInputTypeInfo.bActionRequiresHold = true;
    Table->AddRow(TEXT("Hold"), Row);
    Row.KeyboardInputTypeInfo.bActionRequiresHold = false;
    Table->AddRow(TEXT("Tap"), Row);
    FDataTableRowHandle Hold; Hold.DataTable = Table.Get(); Hold.RowName = TEXT("Hold");
    FDataTableRowHandle Tap; Tap.DataTable = Table.Get(); Tap.RowName = TEXT("Tap");
    UDataTable* Original = LoadObject<UDataTable>(nullptr, TEXT("/Game/UI/Foundation/Input/InputActionDataTable.InputActionDataTable"));
    const TArray<FName> OriginalNames = Original ? Original->GetRowNames() : TArray<FName>();
    Record(TEXT("original_row"), Original && Original->RowStruct == FCommonInputActionData::StaticStruct()
        && OriginalNames.Num() == 421 && Original->FindRow<FCommonInputActionData>(OriginalNames[0], TEXT("ActionWidgetFlow")));
    if (Original && OriginalNames.Num())
    {
        FDataTableRowHandle OriginalRow; OriginalRow.DataTable = Original; OriginalRow.RowName = OriginalNames[0];
        Widget->SetInputAction(OriginalRow);
        Record(TEXT("original_handle"), Widget->InputActions.Num() == 1 && Widget->InputActions[0].DataTable == Original
            && Widget->InputActions[0].RowName == OriginalNames[0]);
    }
    else Record(TEXT("original_handle"), false);
    Widget->SetInputAction(Hold);
    Record(TEXT("before_slate"), Widget->GetVisibility() == ESlateVisibility::Collapsed && Widget->GetIcon().GetResourceObject() == Texture.Get());
    TSharedRef<SWidget> Slate = Widget->TakeWidget();
    TSharedPtr<SBox> Box = Widget->Box();
    TSharedPtr<SImage> Rim = Widget->Rim(), Icon = Widget->Icon(), Progress = Widget->Progress();
    FChildren* BoxChildren = Box.IsValid() ? Box->GetChildren() : nullptr;
    TSharedPtr<SWidget> Overlay = BoxChildren && BoxChildren->Num() == 1 ? BoxChildren->GetChildAt(0) : nullptr;
    FChildren* Children = Overlay.IsValid() ? Overlay->GetChildren() : nullptr;
    Record(TEXT("slate_order"), Box.IsValid() && Rim.IsValid() && Icon.IsValid() && Progress.IsValid() && Children && Children->Num() == 3
        && &Children->GetChildAt(0).Get() == Rim.Get() && &Children->GetChildAt(1).Get() == Icon.Get()
        && &Children->GetChildAt(2).Get() == Progress.Get());
    Record(TEXT("hold_visible"), Widget->GetVisibility() == ESlateVisibility::SelfHitTestInvisible
        && Progress.IsValid() && Progress->GetVisibility() == EVisibility::SelfHitTestInvisible
        && Icon.IsValid() && Widget->CachedBrush().GetResourceObject() == Texture.Get()
        && Widget->IsHeldAction() && Widget->GetDisplayText().ToString() == TEXT("Hold action"));
    Record(TEXT("subscribed"), Input->NativeInputMethodChanged.IsBoundToObject(Widget.Get()));
    FCommonActionBindingRecord Binding{}; Binding.Owner = Owner.Get();
    int32 Key = 123456789;
    while (Key < MAX_int32 && CommonActionBindingData::Find(Key)) ++Key;
    const bool Available = !CommonActionBindingData::Find(Key);
    Record(TEXT("registry_available"), Available);
    if (!Available) {
        ShowKeys->SetWithCurrentPriority(SavedShow); Input->CurrentInputType = SavedType;
        Controller->InputType = SavedControllerType; Controller->InputBrushDataMap = SavedMap;
        Settings->CommonInputPlatformData[0] = SavedPlatform; Settings->CurrentPlatform = SavedCurrent;
        return;
    }
    CommonActionBindingData::Add(Key, Binding);
    Widget->NativeSetBindingKey(Key);
    Record(TEXT("borrow_binding"), Widget->NativeBindingKey == Key && CommonActionBindingData::Find(Key)
        && Input->NativeInputMethodChanged.IsBoundToObject(Widget.Get()));
    Widget->SetInputAction(Tap);
    Record(TEXT("single_detaches"), Widget->NativeBindingKey == -1 && CommonActionBindingData::Find(Key)
        && Widget->InputActions.Num() == 1 && Widget->InputActions[0].RowName == Tap.RowName
        && Progress->GetVisibility() == EVisibility::Collapsed && !Widget->IsHeldAction()
        && Widget->GetDisplayText().ToString() == TEXT("Tap action"));
    Widget->NativeSetBindingKey(Key);
    Widget->SetInputActions({Hold});
    Record(TEXT("array_detaches"), Widget->NativeBindingKey == -1 && CommonActionBindingData::Find(Key)
        && Widget->InputActions.Num() == 1 && Widget->InputActions[0].RowName == Hold.RowName
        && Progress->GetVisibility() == EVisibility::SelfHitTestInvisible);
    Widget->OnInputMethodChanged.AddDynamic(Widget.Get(), &UActionWidgetFlowWidget::ObserveInputMethodChanged);
    ShowKeys->SetWithCurrentPriority(0);
    Input->NativeInputMethodChanged.Broadcast(ECommonInputType::Gamepad);
    Record(TEXT("showkeys_off"), Widget->GetVisibility() == ESlateVisibility::Collapsed
        && Widget->bObservedInputMethod && Widget->bObservedGamepad
        && Widget->ObservedVisibility == ESlateVisibility::Collapsed);
    ShowKeys->SetWithCurrentPriority(1); Widget->NativeRefreshInputAction();
    Record(TEXT("showkeys_on"), Widget->GetVisibility() == ESlateVisibility::SelfHitTestInvisible);
    Widget->NativeSetBindingKey(Key);
    Widget->ReleaseSlateResources(true);
    Record(TEXT("release"), !Input->NativeInputMethodChanged.IsBoundToObject(Widget.Get())
        && !Widget->Box().IsValid() && !Widget->Icon().IsValid() && !Widget->Progress().IsValid()
        && CommonActionBindingData::Find(Key) && Widget->NativeBindingKey == Key);
    Record(TEXT("registry_cleanup"), CommonActionBindingData::Remove(Key) == 1);
    ShowKeys->SetWithCurrentPriority(SavedShow);
    Input->CurrentInputType = SavedType;
    Controller->InputType = SavedControllerType; Controller->InputBrushDataMap = SavedMap;
    Settings->CommonInputPlatformData[0] = SavedPlatform; Settings->CurrentPlatform = SavedCurrent;
    Record(TEXT("restored"), Input->CurrentInputType == SavedType && ShowKeys->GetInt() == SavedShow);
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
        if (!World || !World->IsGameWorld() || !Player || !PC || PC->Player != Player || PC->GetWorld() != World
            || Player->GetGameInstance() != GI || GI->GetWorld() != World) continue;
        Run(Player, PC); Finish(); return;
    }
}
}
void RegisterCommonActionWidgetFlowProbeDriver()
{
    using namespace ActionWidgetFlowDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("CommonActionWidgetFlowProbe"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
