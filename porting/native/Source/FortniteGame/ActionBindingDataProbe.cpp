#include "ActionBindingDataProbe.h"
#include "CommonActionBindingData.h"
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
#include "UObject/GarbageCollection.h"
#include "UObject/StrongObjectPtr.h"

namespace ActionBindingDataDiagnostic
{
int32 Sequence = 0, Clicks = 0;
TArray<float> ProgressValues;
bool Failed = false, Registered = false;
double Deadline = 0;
FDelegateHandle FrameHandle;

void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("BINDDATA v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
}
uint32 Bits(float Value) { uint32 Result; FMemory::Memcpy(&Result, &Value, sizeof(Result)); return Result; }
float Float(uint32 Value) { float Result; FMemory::Memcpy(&Result, &Value, sizeof(Result)); return Result; }

struct FResolver final : ICommonActionBindingDataResolver
{
    FUIInputAction Action;
    FCommonInputActionData Legacy;
    TArray<int32> Calls;
    UObject* ExpectedOwner = nullptr;
    const FCommonActionBindingDescriptor* NestedDescriptor = nullptr;
    int32 NestedId = -1;
    bool Reenter = false;
    const FUIInputAction& ResolveTag(FName Tag) override
    {
        Calls.Add(1);
        check(Tag == FName(TEXT("Probe.Action")));
        if (Reenter)
        {
            Reenter = false;
            NestedId = CommonActionBindingData::BuildRecord(*NestedDescriptor, ExpectedOwner, *this).Id;
        }
        return Action;
    }
    const FCommonInputActionData& ResolveRow(const FDataTableRowHandle& Handle) override
    {
        Calls.Add(2);
        const FCommonInputActionData* Row = Handle.GetRow<FCommonInputActionData>(TEXT("ActionBindingDataProbe"));
        check(Row);
        return *Row;
    }
    int32 ResolveGamepadIndex(UObject* Owner) override { Calls.Add(3); check(Owner == ExpectedOwner); return 2; }
};

struct FInspectProgress : FCommonActionBindingProgressMulticast
{
    explicit FInspectProgress(const FCommonActionBindingProgressMulticast& Other) : FCommonActionBindingProgressMulticast(Other) {}
    int32 Count() const { return GetInvocationList().Num(); }
};

void Run()
{
    Record(TEXT("begin"), true);
    TStrongObjectPtr<UActionBindingDataProbeReceiver> Receiver(NewObject<UActionBindingDataProbeReceiver>());
    TStrongObjectPtr<UDataTable> Table(NewObject<UDataTable>(GetTransientPackage(), TEXT("BindingProbeTable")));
    Record(TEXT("context"), Receiver.IsValid() && Table.IsValid());
    if (!Receiver.IsValid() || !Table.IsValid()) return;
    Record(TEXT("reflected_types"), FUIInputAction::StaticStruct()->GetStructureSize() == 48
        && FUIActionKeyMapping::StaticStruct()->GetStructureSize() == 32
        && FCommonInputActionData::StaticStruct()->GetStructureSize() == 1064
        && FCommonInputTypeInfo::StaticStruct()->GetStructureSize() == 168);

    FResolver Resolver; Resolver.ExpectedOwner = Receiver.Get();
    Resolver.Action.DefaultDisplayName = FText::FromString(TEXT("Tagged display"));
    auto Add = [&](FKey Key, uint32 Time) {
        FUIActionKeyMapping& Mapping = Resolver.Action.KeyMappings.AddDefaulted_GetRef();
        Mapping.Key = Key; Mapping.HoldTime = Float(Time);
    };
    Add(EKeys::SpaceBar, 0); Add(EKeys::SpaceBar, 0x7fc12345u);
    Add(EKeys::Enter, 0xbf800000u); Add(FKey(), 0x3f800000u);
    Add(EKeys::Gamepad_FaceButton_Bottom, 0x3e800000u); Add(EKeys::Enter, 0x80000000u);
    Add(EKeys::SpaceBar, 0x7f800000u); Add(EKeys::SpaceBar, 0xff800000u);

    FCommonActionBindingDescriptor Descriptor;
    Descriptor.Tag = FName(TEXT("Probe.Action")); Descriptor.InputMode = 2; Descriptor.Event = 4;
    Descriptor.Flag20 = 0x81; Descriptor.Flag21 = 0x72; Descriptor.Flag22 = 0x63;
    Descriptor.Click.BindUObject(Receiver.Get(), &UActionBindingDataProbeReceiver::Click);
    Descriptor.Progress.BindUObject(Receiver.Get(), &UActionBindingDataProbeReceiver::Progress);
    FCommonActionBindingRecord Tagged = CommonActionBindingData::BuildRecord(Descriptor, Receiver.Get(), Resolver);
    Record(TEXT("tag_fields"), Tagged.Name == Descriptor.Tag && Tagged.Event == 4 && Tagged.InputMode == 2
        && Tagged.Flag20 == 0x81 && Tagged.Flag21 == 0x72 && Tagged.Flag22 == 0x63
        && Tagged.Owner.Get() == Receiver.Get() && Tagged.Display.ToString() == TEXT("Tagged display")
        && Tagged.State == -1.0 && !Tagged.Route.IsValid());
    Record(TEXT("tag_key_partition"), Tagged.Press.Num() == 4 && Tagged.Hold.Num() == 3
        && Tagged.Press[0].Key == EKeys::SpaceBar && Tagged.Press[1].Key == EKeys::Enter
        && Tagged.Press[2].Key == EKeys::Enter && Tagged.Press[3].Key == EKeys::SpaceBar
        && Tagged.Hold[0].Key == EKeys::SpaceBar && Tagged.Hold[1].Key == EKeys::Gamepad_FaceButton_Bottom);
    Record(TEXT("tag_raw_float_bits"), Tagged.Press.Num() == 4 && Tagged.Hold.Num() == 3
        && Bits(Tagged.Press[2].Time) == 0x80000000u && Bits(Tagged.Hold[0].Time) == 0x7fc12345u
        && Bits(Tagged.Hold[2].Time) == 0x7f800000u && Bits(Tagged.Press[3].Time) == 0xff800000u);

    Descriptor.DisplayOverride = FText::FromString(TEXT("Explicit override"));
    FCommonActionBindingRecord Override = CommonActionBindingData::BuildRecord(Descriptor, Receiver.Get(), Resolver);
    Record(TEXT("override_text"), Override.Display.ToString() == TEXT("Explicit override"));
    Descriptor.Row.DataTable = Table.Get(); Descriptor.Row.RowName = TEXT("RowA");
    FCommonActionBindingRecord Hybrid = CommonActionBindingData::BuildRecord(Descriptor, Receiver.Get(), Resolver);
    Record(TEXT("legacy_name_priority"), Hybrid.Name == FName(TEXT("Legacy_BindingProbeTable_RowA"))
        && Hybrid.Row.DataTable == Table.Get() && Hybrid.Row.RowName == FName(TEXT("RowA")));
    Descriptor.Row.DataTable = nullptr;
    FCommonActionBindingRecord InvalidName = CommonActionBindingData::BuildRecord(Descriptor, Receiver.Get(), Resolver);
    Record(TEXT("invalid_legacy_name"), InvalidName.Name.IsNone());

    Descriptor.Tag = NAME_None; Descriptor.Row.DataTable = Table.Get(); Descriptor.DisplayOverride = FText::GetEmpty();
    Resolver.Legacy.DisplayName = FText::FromString(TEXT("Legacy display"));
    Resolver.Legacy.HoldDisplayName = FText::FromString(TEXT("Unused hold display"));
    Resolver.Legacy.KeyboardInputTypeInfo.Key = EKeys::SpaceBar;
    Resolver.Legacy.KeyboardInputTypeInfo.bActionRequiresHold = true;
    Resolver.Legacy.KeyboardInputTypeInfo.HoldTime = Float(0x7fc12345u);
    Resolver.Legacy.TouchInputTypeInfo.Key = EKeys::TouchKeys[0];
    Resolver.Legacy.TouchInputTypeInfo.HoldTime = 1.0f;
    Resolver.Legacy.GamepadInputTypeInfos[0].Key = EKeys::A;
    Resolver.Legacy.GamepadInputTypeInfos[2].Key = EKeys::Gamepad_FaceButton_Bottom;
    Resolver.Legacy.GamepadInputTypeInfos[2].bActionRequiresHold = true;
    Resolver.Legacy.GamepadInputTypeInfos[2].HoldTime = 0.75f;
    Table->RowStruct = FCommonInputActionData::StaticStruct();
    Table->AddRow(FName(TEXT("RowA")), Resolver.Legacy);
    Resolver.Calls.Reset();
    FCommonActionBindingRecord Legacy = CommonActionBindingData::BuildRecord(Descriptor, Receiver.Get(), Resolver);
    Record(TEXT("legacy_fields"), Legacy.Name == FName(TEXT("Legacy_BindingProbeTable_RowA"))
        && Legacy.Display.ToString() == TEXT("Legacy display") && Legacy.Id == Tagged.Id + 4);
    Record(TEXT("legacy_key_order"), Legacy.Press.Num() == 2 && Legacy.Hold.Num() == 1
        && Legacy.Press[0].Key == EKeys::SpaceBar && Legacy.Press[1].Key == EKeys::TouchKeys[0]
        && Legacy.Hold[0].Key == EKeys::Gamepad_FaceButton_Bottom);
    Record(TEXT("legacy_float_bits"), Legacy.Press.Num() == 2 && Legacy.Hold.Num() == 1
        && Bits(Legacy.Press[0].Time) == 0x7fc12345u && Bits(Legacy.Press[1].Time) == 0
        && Bits(Legacy.Hold[0].Time) == 0x3f400000u);
    Record(TEXT("resolver_order"), Resolver.Calls.Num() == 2 && Resolver.Calls[0] == 2 && Resolver.Calls[1] == 3);

    FCommonActionBindingRecord Copy = Tagged;
    Tagged.Press[0].Time = -99.0f;
    Record(TEXT("copy_arrays"), Copy.Press.GetData() != Tagged.Press.GetData()
        && Copy.Hold.GetData() != Tagged.Hold.GetData() && Bits(Copy.Press[0].Time) == 0
        && Copy.Display.ToString() == TEXT("Tagged display"));
    Copy.Click.ExecuteIfBound(); Copy.ProgressListeners.Broadcast(0.125f);
    Record(TEXT("delegate_copy"), Clicks == 1 && ProgressValues.Num() == 1 && Bits(ProgressValues[0]) == 0x3e000000u);
    const int32 Key = Tagged.Id;
    CommonActionBindingData::Add(Key, Copy);
    FCommonActionBindingRecord* Stored = CommonActionBindingData::Find(Key);
    Record(TEXT("map_find"), Stored && Stored->Id == Key && Stored->Press.GetData() != Copy.Press.GetData());
    struct FToken {};
    TSharedPtr<FToken> Token = MakeShared<FToken>(); TWeakPtr<FToken> WeakToken = Token;
    FCommonActionBindingRecord TokenRecord = Copy;
    TokenRecord.Click = FSimpleDelegate::CreateLambda([Token] { (void)Token; });
    CommonActionBindingData::Add(Key, TokenRecord); TokenRecord.Click.Unbind(); Token.Reset();
    const bool Retained = WeakToken.IsValid();
    CommonActionBindingData::Add(Key, Legacy); Stored = CommonActionBindingData::Find(Key);
    Record(TEXT("map_replace_release"), Retained && !WeakToken.IsValid() && Stored && Stored->Id == Legacy.Id && Stored->Name == Legacy.Name);
    const int32 Removed = CommonActionBindingData::Remove(Key);
    Record(TEXT("map_remove"), Removed == 1 && !CommonActionBindingData::Find(Key) && CommonActionBindingData::Remove(Key) == 0);

    uint32 Counter = 0;
    Record(TEXT("id_zero"), CommonActionBindingData::AllocateIdBits(Counter) == 0 && Counter == 1 && Tagged.Id == 0);
    Counter = 0x7fffffffu;
    const int32 LastPositive = CommonActionBindingData::AllocateIdBits(Counter);
    const int32 FirstNegative = CommonActionBindingData::AllocateIdBits(Counter);
    Record(TEXT("id_signed_wrap"), LastPositive == MAX_int32 && FirstNegative == MIN_int32 && Counter == 0x80000001u);
    Counter = 0xffffffffu;
    const int32 LastBits = CommonActionBindingData::AllocateIdBits(Counter);
    const int32 FirstBits = CommonActionBindingData::AllocateIdBits(Counter);
    Record(TEXT("id_full_wrap"), LastBits == -1 && FirstBits == 0 && Counter == 1);
    Descriptor.Tag = FName(TEXT("Probe.Action")); Descriptor.Row = FDataTableRowHandle();
    Resolver.NestedDescriptor = &Descriptor; Resolver.Reenter = true;
    FCommonActionBindingRecord Reentrant = CommonActionBindingData::BuildRecord(Descriptor, Receiver.Get(), Resolver);
    Record(TEXT("reentrant_id_order"), Resolver.NestedId == Reentrant.Id + 1);
    TWeakObjectPtr<UObject> WeakOwner = Receiver.Get();
    Record(TEXT("weak_owner_live"), Copy.Owner.Get() == Receiver.Get() && WeakOwner.IsValid());
    Receiver.Reset(); CollectGarbage(RF_NoFlags, true);
    Record(TEXT("weak_owner_expired"), !WeakOwner.IsValid() && !Copy.Owner.IsValid() && !Copy.Click.IsBound());
    // A dead weak-bound delegate still has a cloneable delegate instance.
    FCommonActionBindingRecord Dead = CommonActionBindingData::BuildRecord(Descriptor, nullptr, Resolver);
    FInspectProgress Inspected(Dead.ProgressListeners);
    const int32 OldClicks = Clicks, OldProgress = ProgressValues.Num();
    Dead.Click.ExecuteIfBound(); Dead.ProgressListeners.Broadcast(0.5f);
    Record(TEXT("weak_callbacks_expired"), Inspected.Count() == 1 && Clicks == OldClicks && ProgressValues.Num() == OldProgress);
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
        if (!World || !World->IsGameWorld() || !GI || !Player || !PC || PC->Player != Player || PC->GetWorld() != World
            || Player->GetGameInstance() != GI || GI->GetWorld() != World) continue;
        Run(); Finish(); return;
    }
}
}

void UActionBindingDataProbeReceiver::Click() { ++ActionBindingDataDiagnostic::Clicks; }
void UActionBindingDataProbeReceiver::Progress(float Value) { ActionBindingDataDiagnostic::ProgressValues.Add(Value); }
void RegisterActionBindingDataProbeDriver()
{
    using namespace ActionBindingDataDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("ActionBindingDataProbe"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
