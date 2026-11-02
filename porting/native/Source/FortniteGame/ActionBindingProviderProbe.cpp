#include "CommonActionBindingProviders.h"
#include "UCommonUIInputSettings.h"
#include "UCommonUIActionRouter.h"
#include "UCommonUserWidget.h"
#include "UCommonInputSubsystem.h"
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

namespace ActionBindingProviderDiagnostic
{
int32 Sequence = 0;
bool Failed = false, Registered = false;
double Deadline = 0;
FDelegateHandle FrameHandle;

void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("BINDPROVIDER v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
}

bool ActionsEqual(const TArray<FUIInputAction>& A, const TArray<FUIInputAction>& B)
{
    if (A.Num() != B.Num()) return false;
    for (int32 Index = 0; Index < A.Num(); ++Index)
        if (!FUIInputAction::StaticStruct()->CompareScriptStruct(&A[Index], &B[Index], 0)) return false;
    return true;
}

void Run(ULocalPlayer* Player, APlayerController* Controller)
{
    Record(TEXT("begin"), true);
    UCommonUIActionRouter* Router = Player->GetSubsystem<UCommonUIActionRouter>();
    UCommonInputSubsystem* Input = Player->GetSubsystem<UCommonInputSubsystem>();
    TStrongObjectPtr<UCommonUserWidget> Widget(CreateWidget<UCommonUserWidget>(Controller, UCommonUserWidget::StaticClass()));
    Record(TEXT("context"), Router && Input && Widget.IsValid() && Router->GetOuter() == Player
        && Input->GetOuter() == Router->GetOuter() && Widget->GetOwningLocalPlayer() == Player);
    if (!Router || !Input || !Widget.IsValid()) return;

    UCommonUIInputSettings* Settings = GetMutableDefault<UCommonUIInputSettings>();
    FNameProperty* TagName = FindFProperty<FNameProperty>(FUIActionTag::StaticStruct(), TEXT("TagName"));
    Record(TEXT("settings"), Settings == GetDefault<UCommonUIInputSettings>() && TagName);
    if (!Settings || !TagName) return;
    const TArray<FUIInputAction> SavedActions = Settings->InputActions;
    const TArray<FUIInputAction> SavedOverrides = Settings->ActionOverrides;
    const ECommonGamepadType SavedGamepad = Input->GamepadInputType;
    const ECommonInputType SavedType = Input->CurrentInputType;
    const FName Tag(TEXT("Probe.Binding.Provider"));
    auto MakeAction = [&](const TCHAR* Display, FKey Key) {
        FUIInputAction Action;
        TagName->SetPropertyValue_InContainer(&Action.ActionTag, Tag);
        Action.DefaultDisplayName = FText::FromString(Display);
        FUIActionKeyMapping& Mapping = Action.KeyMappings.AddDefaulted_GetRef();
        Mapping.Key = Key; Mapping.HoldTime = 0.0f;
        return Action;
    };
    Settings->InputActions = {MakeAction(TEXT("First"), EKeys::SpaceBar), MakeAction(TEXT("Second"), EKeys::Enter)};
    Settings->ActionOverrides = {MakeAction(TEXT("Override"), EKeys::Escape)};
    FCommonActionBindingEngineResolver Resolver;
    Record(TEXT("tag_first_match"), &Resolver.ResolveTag(Tag) == &Settings->InputActions[0]
        && Resolver.ResolveTag(Tag).DefaultDisplayName.ToString() == TEXT("First"));
    Settings->InputActions.Swap(0, 1);
    Record(TEXT("tag_live_order"), &Resolver.ResolveTag(Tag) == &Settings->InputActions[0]
        && Resolver.ResolveTag(Tag).DefaultDisplayName.ToString() == TEXT("Second"));
    FCommonActionBindingDescriptor Descriptor{};
    Descriptor.Tag = Tag;
    const FCommonActionBindingRecord Tagged = CommonActionBindingData::BuildRecord(Descriptor, Widget.Get(), Resolver);
    Record(TEXT("tag_build"), Tagged.Name == Tag && Tagged.Display.ToString() == TEXT("Second")
        && Tagged.Press.Num() == 1 && Tagged.Press[0].Key == EKeys::Enter && Tagged.Hold.Num() == 0
        && Tagged.Owner.Get() == Widget.Get());

    TStrongObjectPtr<UDataTable> Table(NewObject<UDataTable>(GetTransientPackage(), TEXT("BindingProviderTable")));
    Table->RowStruct = FCommonInputActionData::StaticStruct();
    FCommonInputActionData Row;
    Row.DisplayName = FText::FromString(TEXT("Legacy provider"));
    Row.KeyboardInputTypeInfo.Key = EKeys::SpaceBar;
    Row.KeyboardInputTypeInfo.bActionRequiresHold = false;
    Row.TouchInputTypeInfo.Key = EKeys::TouchKeys[0];
    Row.TouchInputTypeInfo.bActionRequiresHold = false;
    const FKey Keys[] = {EKeys::Gamepad_FaceButton_Bottom, EKeys::Gamepad_FaceButton_Right,
        EKeys::Gamepad_FaceButton_Left, EKeys::Gamepad_FaceButton_Top};
    for (int32 Index = 0; Index < 4; ++Index)
    {
        Row.GamepadInputTypeInfos[Index].Key = Keys[Index];
        Row.GamepadInputTypeInfos[Index].bActionRequiresHold = false;
    }
    const FName RowName(TEXT("ProviderRow"));
    Table->AddRow(RowName, Row);
    Descriptor.Tag = NAME_None; Descriptor.Row.DataTable = Table.Get(); Descriptor.Row.RowName = RowName;
    Record(TEXT("row_identity"), &Resolver.ResolveRow(Descriptor.Row)
        == Table->FindRow<FCommonInputActionData>(RowName, TEXT("ActionBindingProviderProbe")));
    UDataTable* Original = LoadObject<UDataTable>(nullptr, TEXT("/Game/UI/Foundation/Input/InputActionDataTable.InputActionDataTable"));
    const TArray<FName> OriginalNames = Original ? Original->GetRowNames() : TArray<FName>();
    FDataTableRowHandle OriginalHandle;
    OriginalHandle.DataTable = Original;
    if (OriginalNames.Num()) OriginalHandle.RowName = OriginalNames[0];
    Record(TEXT("original_row"), Original && OriginalNames.Num() == 421
        && &Resolver.ResolveRow(OriginalHandle) == Original->FindRow<FCommonInputActionData>(OriginalHandle.RowName, TEXT("ActionBindingProviderProbe")));

    // Query the adapter with unnamed bytes too. Do not index the four-element
    // row with those values: the original constructor requires a valid index.
    const uint8 Bytes[] = {0, 3, 1, 2, 255, 128};
    const TCHAR* IndexEvents[] = {TEXT("index_0"), TEXT("index_3"), TEXT("index_1"), TEXT("index_2"), TEXT("index_255"), TEXT("index_128")};
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Bytes); ++Index)
    {
        Input->GamepadInputType = static_cast<ECommonGamepadType>(Bytes[Index]);
        Record(IndexEvents[Index], Resolver.ResolveGamepadIndex(Widget.Get()) == Bytes[Index]);
    }
    const TCHAR* BuildEvents[] = {TEXT("legacy_0"), TEXT("legacy_1"), TEXT("legacy_2"), TEXT("legacy_3")};
    for (int32 Index = 0; Index < 4; ++Index)
    {
        Input->GamepadInputType = static_cast<ECommonGamepadType>(Index);
        Input->CurrentInputType = static_cast<ECommonInputType>(Index % 3);
        const FCommonActionBindingRecord Legacy = CommonActionBindingData::BuildRecord(Descriptor, Widget.Get(), Resolver);
        Record(BuildEvents[Index], Legacy.Press.Num() == 3 && Legacy.Hold.Num() == 0
            && Legacy.Press[0].Key == EKeys::SpaceBar && Legacy.Press[1].Key == EKeys::TouchKeys[0]
            && Legacy.Press[2].Key == Keys[Index] && Legacy.Display.ToString() == TEXT("Legacy provider")
            && Legacy.Owner.Get() == Widget.Get());
    }
    Settings->InputActions = SavedActions; Settings->ActionOverrides = SavedOverrides;
    Input->GamepadInputType = SavedGamepad; Input->CurrentInputType = SavedType;
    Record(TEXT("restored"), ActionsEqual(Settings->InputActions, SavedActions)
        && ActionsEqual(Settings->ActionOverrides, SavedOverrides)
        && Input->GamepadInputType == SavedGamepad && Input->CurrentInputType == SavedType);
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
        UWorld* World = Context.World();
        UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
        ULocalPlayer* Player = GI ? GI->GetFirstGamePlayer() : nullptr;
        APlayerController* PC = Player ? Player->GetPlayerController(World) : nullptr;
        if (!World || !World->IsGameWorld() || !Player || !PC || PC->Player != Player
            || PC->GetWorld() != World || Player->GetGameInstance() != GI || GI->GetWorld() != World) continue;
        Run(Player, PC); Finish(); return;
    }
}
}

void RegisterActionBindingProviderProbeDriver()
{
    using namespace ActionBindingProviderDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("ActionBindingProviderProbe"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
