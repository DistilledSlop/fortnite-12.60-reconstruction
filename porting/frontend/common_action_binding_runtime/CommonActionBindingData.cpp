#include "CommonActionBindingRoute.h"
#include "CommonActionBindingData.h"
#include "CommonActionBindingProviders.h"
#include "Components/Widget.h"

DEFINE_LOG_CATEGORY(LogUIActionRouter);

namespace
{
    uint32 NextIdBits = 0;
    TMap<int32, FCommonActionBindingRecord> Records;

    int32 AllocateId()
    {
        return CommonActionBindingData::AllocateIdBits(NextIdBits);
    }

    FName DeriveName(const FCommonActionBindingDescriptor& Descriptor)
    {
        if (Descriptor.Row.DataTable)
        {
            return FName(*FString::Printf(TEXT("Legacy_%s_%s"),
                *Descriptor.Row.DataTable->GetName(), *Descriptor.Row.RowName.ToString()));
        }
        return Descriptor.Row.RowName.IsNone() ? Descriptor.Tag : NAME_None;
    }

    // Integer classification avoids fast-math folding away the two different NaN cases.
    bool IsHold(float Time, bool bTag)
    {
        uint32 Bits;
        FMemory::Memcpy(&Bits, &Time, sizeof(Bits));
        const uint32 Magnitude = Bits & 0x7fffffffu;
        const bool bNaN = Magnitude > 0x7f800000u;
        if (bNaN) return bTag;
        return Time > 0.0f;
    }

    void AddKey(FCommonActionBindingRecord& Record, const FKey& Key, float Time, bool bTag)
    {
        if (!Key.IsValid()) return;
        auto& Array = IsHold(Time, bTag) ? Record.Hold : Record.Press;
        FCommonActionBindingKeyTime& Entry = Array.AddDefaulted_GetRef();
        Entry.Key = Key;
        FMemory::Memcpy(&Entry.Time, &Time, sizeof(Time));
    }

    void AddLegacy(FCommonActionBindingRecord& Record, const FCommonInputTypeInfo& Info)
    {
        const float Time = Info.bActionRequiresHold ? Info.HoldTime : 0.0f;
        AddKey(Record, Info.Key, Time, false);
    }
}

int32 CommonActionBindingData::AllocateIdBits(uint32& Counter)
{
    const uint32 Bits = Counter++;
    int32 Id;
    FMemory::Memcpy(&Id, &Bits, sizeof(Id));
    return Id;
}

FCommonActionBindingRecord CommonActionBindingData::BuildRecord(
    const FCommonActionBindingDescriptor& Descriptor, UObject* Owner,
    ICommonActionBindingDataResolver& Resolver)
{
    FCommonActionBindingRecord Record;
    Record.Name = DeriveName(Descriptor);
    Record.Event = Descriptor.Event;
    Record.Flag21 = Descriptor.Flag21;
    Record.Flag20 = Descriptor.Flag20;
    Record.Owner = Owner;
    Record.InputMode = Descriptor.InputMode;
    Record.Flag22 = Descriptor.Flag22;
    Record.Display = Descriptor.DisplayOverride;
    Record.Click = Descriptor.Click;
    Record.Id = AllocateId();
    Record.Row = Descriptor.Row;
    Record.State = -1.0;
    Record.ProgressListeners.Add(Descriptor.Progress);

    if (!Descriptor.Tag.IsNone())
    {
        const FUIInputAction& Action = Resolver.ResolveTag(Descriptor.Tag);
        if (Record.Display.IsEmpty()) Record.Display = Action.DefaultDisplayName;
        for (const FUIActionKeyMapping& Mapping : Action.KeyMappings)
            AddKey(Record, Mapping.Key, Mapping.HoldTime, true);
    }
    else
    {
        const FCommonInputActionData& Row = Resolver.ResolveRow(Record.Row);
        if (Record.Display.IsEmpty()) Record.Display = Row.DisplayName;
        AddLegacy(Record, Row.KeyboardInputTypeInfo);
        AddLegacy(Record, Row.TouchInputTypeInfo);
        const int32 Index = Resolver.ResolveGamepadIndex(Owner);
        AddLegacy(Record, Row.GamepadInputTypeInfos[Index]);
    }
    return Record;
}

FCommonActionBindingRecord* CommonActionBindingData::Find(int32 Key) { return Records.Find(Key); }
FCommonActionBindingRecord& CommonActionBindingData::Add(int32 Key, const FCommonActionBindingRecord& Record) { return Records.Add(Key, Record); }
int32 CommonActionBindingData::Remove(int32 Key) { return Records.Remove(Key); }

int32 CommonActionBindingData::Create(const FCommonActionBindingDescriptor& Descriptor, UWidget* Owner)
{
    if (DeriveName(Descriptor).IsNone())
    {
        UE_LOG(LogUIActionRouter, Error, TEXT("Cannot create action binding for widget [%s] - no action provided."), *Owner->GetFName().ToString());
        return -1;
    }
    if (!Descriptor.Click.IsBound())
    {
        UE_LOG(LogUIActionRouter, Error, TEXT("Cannot bind widget [%s] to action [%s] - empty handler delegate."), *Owner->GetFName().ToString(), *DeriveName(Descriptor).ToString());
        return -1;
    }
    FCommonActionBindingEngineResolver Resolver;
    if (!Descriptor.Tag.IsNone() && !Resolver.FindTag(Descriptor.Tag))
    {
        UE_LOG(LogUIActionRouter, Error, TEXT("Cannot bind widget [%s] to action [%s] - provided tag does not map to an existing UI input action. It can be added under Project Settings->UI Input."), *Owner->GetFName().ToString(), *DeriveName(Descriptor).ToString());
        return -1;
    }
    if ((Descriptor.Row.DataTable || !Descriptor.Row.RowName.IsNone()) && !Resolver.FindRow(Descriptor.Row))
    {
        UE_LOG(LogUIActionRouter, Error, TEXT("Cannot bind widget [%s] to action [%s] - provided legacy data table row does not resolve to valid data."), *Owner->GetFName().ToString(), *DeriveName(Descriptor).ToString());
        return -1;
    }
    for (const auto& Pair : Records)
    {
        const FCommonActionBindingRecord& Existing = Pair.Value;
        if (Existing.Name == DeriveName(Descriptor) && Existing.Event == Descriptor.Event && Existing.Owner.Get() == Owner)
        {
            const TCHAR* EventName = TEXT("Invalid");
            switch (Descriptor.Event)
            {
                case 0: EventName = TEXT("Pressed"); break;
                case 1: EventName = TEXT("Released"); break;
                case 2: EventName = TEXT("Repeat"); break;
                case 3: EventName = TEXT("DoubleClick"); break;
                case 4: EventName = TEXT("Axis"); break;
            }
            UE_LOG(LogUIActionRouter, Error, TEXT("Widget [%s] is already bound to action [%s] [%s]! A given widget can only bind the same action once. Unregister existing binding first if you wish to change any aspect of the binding"), *Owner->GetFName().ToString(), *Existing.Name.ToString(), EventName);
            return -1;
        }
    }
    FCommonActionBindingRecord Record = BuildRecord(Descriptor, Owner, Resolver);
    Records.Add(Record.Id, Record);
    return Record.Id;
}

// Include CommonActionBindingRoute.h in the existing registry translation unit.
// This body uses that translation unit's existing Records map, not another map.
void CommonActionBindingData::CompactInvalidBindings()
{
    for (auto It = Records.CreateIterator(); It; ++It)
    {
        auto& Record = It.Value();
        if (Record.Owner.IsValid() && Record.Click.IsBound()) continue;
        if (Record.Route.IsValid())
        {
            const auto Route = Record.Route.Pin();
            Route->Disassociate(It.Key());
        }
        It.RemoveCurrent();
    }
}
