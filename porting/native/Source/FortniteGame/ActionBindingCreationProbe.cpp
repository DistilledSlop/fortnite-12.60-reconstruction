#include "CommonActionBindingCreations.h"
#include "ActionBindingDataProbe.h"
#include "UObject/GarbageCollection.h"
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

namespace ActionBindingCreationDiagnostic
{
int32 Sequence = 0;
bool Failed = false, Registered = false;
double Deadline = 0;
FDelegateHandle FrameHandle;

void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("BINDCREATE v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
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
    TStrongObjectPtr<UCommonUserWidget> Owner(CreateWidget<UCommonUserWidget>(Controller, UCommonUserWidget::StaticClass(), TEXT("BindingCreateOwner")));
    TStrongObjectPtr<UCommonUserWidget> Other(CreateWidget<UCommonUserWidget>(Controller, UCommonUserWidget::StaticClass(), TEXT("BindingCreateOther")));
    Record(TEXT("context"), Router && Input && Owner.IsValid() && Other.IsValid()
        && Router->GetOuter() == Player && Owner->GetOwningLocalPlayer() == Player && Other->GetOwningLocalPlayer() == Player);
    if (!Router || !Input || !Owner.IsValid() || !Other.IsValid()) return;
    UCommonUIInputSettings* Settings = GetMutableDefault<UCommonUIInputSettings>();
    FNameProperty* TagName = FindFProperty<FNameProperty>(FUIActionTag::StaticStruct(), TEXT("TagName"));
    Record(TEXT("settings"), Settings && TagName);
    if (!Settings || !TagName) return;
    const TArray<FUIInputAction> SavedActions = Settings->InputActions;
    const ECommonGamepadType SavedGamepad = Input->GamepadInputType;
    const FName Tag(TEXT("Probe.Binding.Create")), MissingTag(TEXT("Probe.Binding.Missing"));
    FUIInputAction Action;
    TagName->SetPropertyValue_InContainer(&Action.ActionTag, Tag);
    Action.DefaultDisplayName = FText::FromString(TEXT("Tagged create"));
    FUIActionKeyMapping& Mapping = Action.KeyMappings.AddDefaulted_GetRef();
    Mapping.Key = EKeys::Enter; Mapping.HoldTime = 0.0f;
    Settings->InputActions = {Action};
    TStrongObjectPtr<UDataTable> Table(NewObject<UDataTable>(GetTransientPackage(), TEXT("BindingCreateTable")));
    Table->RowStruct = FCommonInputActionData::StaticStruct();
    FCommonInputActionData Row;
    Row.DisplayName = FText::FromString(TEXT("Legacy create"));
    Row.KeyboardInputTypeInfo.Key = EKeys::SpaceBar;
    Row.TouchInputTypeInfo.Key = EKeys::TouchKeys[0];
    Row.GamepadInputTypeInfos[2].Key = EKeys::Gamepad_FaceButton_Left;
    Table->AddRow(TEXT("Valid"), Row);
    Input->GamepadInputType = static_cast<ECommonGamepadType>(2);
    FCommonActionBindingDescriptor Descriptor{};
    int32 Clicks = 0;
    auto BindClick = [&]() { Descriptor.Click.BindLambda([&Clicks]() { ++Clicks; }); };
    BindClick();
    Record(TEXT("no_action"), CommonActionBindingData::Create(Descriptor, Owner.Get()) == -1);
    Descriptor.Tag = Tag; Descriptor.Click.Unbind();
    Record(TEXT("empty_handler"), CommonActionBindingData::Create(Descriptor, Owner.Get()) == -1);
    TStrongObjectPtr<UActionBindingDataProbeReceiver> Receiver(NewObject<UActionBindingDataProbeReceiver>());
    Descriptor.Click.BindUObject(Receiver.Get(), &UActionBindingDataProbeReceiver::Click);
    Receiver.Reset(); CollectGarbage(RF_NoFlags, true);
    Record(TEXT("stale_handler"), !Descriptor.Click.IsBound() && CommonActionBindingData::Create(Descriptor, Owner.Get()) == -1);
    BindClick(); Descriptor.Tag = MissingTag;
    Record(TEXT("invalid_tag"), CommonActionBindingData::Create(Descriptor, Owner.Get()) == -1);
    Descriptor.Tag = Tag; Descriptor.Row.DataTable = Table.Get(); Descriptor.Row.RowName = TEXT("Missing");
    Record(TEXT("invalid_row"), CommonActionBindingData::Create(Descriptor, Owner.Get()) == -1);
    Descriptor.Tag = MissingTag;
    Record(TEXT("independent_validation"), CommonActionBindingData::Create(Descriptor, Owner.Get()) == -1);
    Descriptor.Tag = Tag; Descriptor.Row = FDataTableRowHandle();
    const int32 First = CommonActionBindingData::Create(Descriptor, Owner.Get());
    const FCommonActionBindingRecord* Found = CommonActionBindingData::Find(First);
    Record(TEXT("first_id_zero"), First == 0 && Found && Found->Id == First && Found->Name == Tag
        && Found->Owner.Get() == Owner.Get() && Found->Display.ToString() == TEXT("Tagged create")
        && Found->Press.Num() == 1 && Found->Press[0].Key == EKeys::Enter && !Found->Route.IsValid());
    Record(TEXT("duplicate"), CommonActionBindingData::Create(Descriptor, Owner.Get()) == -1);
    Descriptor.Flag20 = 7; Descriptor.InputMode = 3; Descriptor.DisplayOverride = FText::FromString(TEXT("Ignored change"));
    Record(TEXT("duplicate_changed_fields"), CommonActionBindingData::Create(Descriptor, Owner.Get()) == -1);
    Descriptor.Event = 1;
    const int32 EventId = CommonActionBindingData::Create(Descriptor, Owner.Get());
    Record(TEXT("different_event"), EventId == 1 && CommonActionBindingData::Find(EventId)->Event == 1);
    Descriptor.Event = 0; Descriptor.DisplayOverride = FText::GetEmpty();
    const int32 OtherId = CommonActionBindingData::Create(Descriptor, Other.Get());
    Record(TEXT("different_owner"), OtherId == 2 && CommonActionBindingData::Find(OtherId)->Owner.Get() == Other.Get());
    Descriptor.Row.DataTable = Table.Get(); Descriptor.Row.RowName = TEXT("Valid");
    const int32 DualId = CommonActionBindingData::Create(Descriptor, Owner.Get());
    Found = CommonActionBindingData::Find(DualId);
    Record(TEXT("tag_and_row"), DualId == 3 && Found && Found->Name == FName(TEXT("Legacy_BindingCreateTable_Valid"))
        && Found->Display.ToString() == TEXT("Tagged create") && Found->Press.Num() == 1 && Found->Press[0].Key == EKeys::Enter);
    Descriptor.Tag = NAME_None;
    Record(TEXT("legacy_duplicate"), CommonActionBindingData::Create(Descriptor, Owner.Get()) == -1);
    Descriptor.Event = 2;
    const int32 LegacyId = CommonActionBindingData::Create(Descriptor, Owner.Get());
    Found = CommonActionBindingData::Find(LegacyId);
    Record(TEXT("legacy_data"), LegacyId == 4 && Found && Found->Display.ToString() == TEXT("Legacy create")
        && Found->Press.Num() == 3 && Found->Press[2].Key == EKeys::Gamepad_FaceButton_Left);
    const bool Removed = CommonActionBindingData::Remove(First) == 1;
    Descriptor.Event = 0; Descriptor.Tag = Tag; Descriptor.Row = FDataTableRowHandle();
    const int32 Recreated = CommonActionBindingData::Create(Descriptor, Owner.Get());
    Record(TEXT("remove_recreate"), Removed && Recreated == 5 && !CommonActionBindingData::Find(First));
    Record(TEXT("no_callbacks"), Clicks == 0);
    Descriptor.Click.Unbind();
    Found = CommonActionBindingData::Find(Recreated);
    if (Found) Found->Click.ExecuteIfBound();
    Record(TEXT("owned_callback"), Clicks == 1);
    bool Clean = true;
    for (int32 Id : {EventId, OtherId, DualId, LegacyId, Recreated})
        Clean = (CommonActionBindingData::Remove(Id) == 1 && !CommonActionBindingData::Find(Id)) && Clean;
    Record(TEXT("cleanup"), Clean);
    Settings->InputActions = SavedActions; Input->GamepadInputType = SavedGamepad;
    Record(TEXT("restored"), ActionsEqual(Settings->InputActions, SavedActions) && Input->GamepadInputType == SavedGamepad);
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

void RegisterActionBindingCreationProbeDriver()
{
    using namespace ActionBindingCreationDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("ActionBindingCreationProbe"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
