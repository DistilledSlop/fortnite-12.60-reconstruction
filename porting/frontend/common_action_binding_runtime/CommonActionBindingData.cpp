#include "CommonActionBindingData.h"

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
