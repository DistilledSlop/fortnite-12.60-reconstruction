// Focused receiver-side wire fixture. Call after production InitGlobalData, before accepting clients.
// Requires FortniteGame -> MantisRuntime dependency so its original package is loaded.
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "FFortGameplayAbilityTargetData_SingleTargetHit.h"
#include "FFortMantisTargetData.h"
#include "Serialization/BitReader.h"
#include "Serialization/BitWriter.h"

// Deliberately limited reference codec, shared by writer and reader. This is NOT a
// network GUID/name parity claim: primitive target framing and payload use production
// serializers; the three null object slots and NAME_None each use an explicit one-bit
// zero marker. The adapter fails closed for any non-null reference or non-None name.
// This avoids FArchive's fatal weak-object operator and its no-op UObject/FName operators.
// The live client test exercises real UPackageMap GUID/name handling.
template <typename TBitArchive>
class TNullTargetDataArchive : public TBitArchive
{
public:
    using TBitArchive::TBitArchive;
    using TBitArchive::operator<<;
    int32 ObjectSlots = 0;
    int32 NameSlots = 0;

    FArchive& operator<<(UObject*& Object) override
    {
        ++ObjectSlots;
        uint8 Present = this->IsSaving() && Object != nullptr ? 1 : 0;
        this->SerializeBits(&Present, 1);
        if (Present != 0) this->SetError();
        if (this->IsLoading()) Object = nullptr;
        return *this;
    }
    FArchive& operator<<(FWeakObjectPtr& WeakObject) override
    {
        UObject* Object = this->IsSaving() ? WeakObject.Get() : nullptr;
        // Reject stale weak references too; fixtures support explicitly null values only.
        if (this->IsSaving() && !WeakObject.IsExplicitlyNull()) this->SetError();
        *this << Object;
        if (this->IsLoading()) WeakObject.Reset();
        return *this;
    }
    FArchive& operator<<(FName& Name) override
    {
        ++NameSlots;
        uint8 Present = this->IsSaving() && !Name.IsNone() ? 1 : 0;
        this->SerializeBits(&Present, 1);
        if (Present != 0) this->SetError();
        if (this->IsLoading()) Name = NAME_None;
        return *this;
    }
};
using FNullTargetDataWriter = TNullTargetDataArchive<FBitWriter>;
using FNullTargetDataReader = TNullTargetDataArchive<FBitReader>;

static bool TargetDataProbeFailure(int32 Line)
{
    UE_LOG(LogTemp, Error, TEXT("Target-data wire probe failed at line %d"), Line);
    return false;
}

bool RunHarvestTargetDataWireProbe()
{
    const TCHAR* Expected[] = {
        TEXT("GameplayAbilityTargetData_SingleTargetHit"),
        TEXT("GameplayAbilityTargetData_LocationInfo"),
        TEXT("GameplayAbilityTargetData_ActorArray"),
        TEXT("GameplayAbilityTargetData"),
        TEXT("FortMantisTargetData"),
        TEXT("FortGameplayAbilityTargetData_SingleTargetHit")
    };
    const auto& Cache = UAbilitySystemGlobals::Get().TargetDataStructCache.ScriptStructs;
    if (Cache.Num() != UE_ARRAY_COUNT(Expected)) return TargetDataProbeFailure(__LINE__);
    for (int32 I = 0; I < Cache.Num(); ++I)
        if (!Cache[I] || Cache[I]->GetName() != Expected[I]) return TargetDataProbeFailure(__LINE__);

    // Build an independent original-client fixture, bypassing custom serializers on the write side.
    for (uint8 Index : {uint8(0), uint8(4), uint8(5)})
    {
        FNullTargetDataWriter Writer(8192, true);
        uint8 UniqueId = 73, Count = 1;
        Writer << UniqueId << Count;
        Writer.SerializeBits(&Index, 8);
        bool Success = false;
        FGameplayAbilityTargetData_SingleTargetHit Stock;
        // FHitResult zeroes these fields; the strict fixture codec requires explicit nulls.
        Stock.HitResult.PhysMaterial.Reset();
        Stock.HitResult.Actor.Reset();
        Stock.HitResult.Component.Reset();
        Stock.HitResult.bBlockingHit = true;
        Stock.HitResult.Time = 0.25f;
        Stock.HitResult.Location = FVector(16, 32, 64);
        Stock.HitResult.ImpactPoint = Stock.HitResult.Location;
        Stock.HitResult.Normal = FVector::UpVector;
        Stock.HitResult.ImpactNormal = Stock.HitResult.Normal;
        if (Index == 0 || Index == 5)
            Stock.NetSerialize(Writer, nullptr, Success);
        int32 Technique = -19, Cartridge = 0x12345678, Weapon = -23456;
        if (Index == 4) Writer << Technique;
        if (Index == 5) Writer << Cartridge << Weapon;
        uint32 Marker = 0xa5c36f19;
        Writer << Marker;
        const int32 ExpectedObjectSlots = Index == 4 ? 0 : 3;
        const int32 ExpectedNameSlots = Index == 4 ? 0 : 1;
        if (Writer.IsError() || Writer.ObjectSlots != ExpectedObjectSlots || Writer.NameSlots != ExpectedNameSlots) return TargetDataProbeFailure(__LINE__);

        FNullTargetDataReader Reader(Writer.GetData(), Writer.GetNumBits());
        FGameplayAbilityTargetDataHandle Received;
        Success = false;
        if (!Received.NetSerialize(Reader, nullptr, Success) || !Success || Reader.IsError()) return TargetDataProbeFailure(__LINE__);
        if (Reader.ObjectSlots != ExpectedObjectSlots || Reader.NameSlots != ExpectedNameSlots) return TargetDataProbeFailure(__LINE__);
        if (Received.UniqueId != UniqueId || Received.Num() != 1) return TargetDataProbeFailure(__LINE__);
        const FGameplayAbilityTargetData* Data = Received.Get(0);
        if (!Data || Data->GetScriptStruct() != Cache[Index]) return TargetDataProbeFailure(__LINE__);
        if (Index == 4 && static_cast<const FFortMantisTargetData*>(Data)->TechniqueDataIndex != Technique) return TargetDataProbeFailure(__LINE__);
        if (Index == 5)
        {
            const auto* Hit = static_cast<const FFortGameplayAbilityTargetData_SingleTargetHit*>(Data);
            if (Hit->CartridgeID != Cartridge || Hit->WeaponInfo != Weapon) return TargetDataProbeFailure(__LINE__);
        }
        if (Index != 4)
        {
            const FHitResult* Hit = Data->GetHitResult();
            if (!Hit || !Hit->bBlockingHit || Hit->Time != Stock.HitResult.Time || Hit->Location != Stock.HitResult.Location)
                return TargetDataProbeFailure(__LINE__);
        }
        uint32 LoadedMarker = 0;
        Reader << LoadedMarker;
        if (Reader.IsError() || LoadedMarker != Marker || Reader.GetPosBits() != Writer.GetNumBits()) return TargetDataProbeFailure(__LINE__);
    }
    return true;
}
