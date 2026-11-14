// Focused receiver-side wire fixture. Call after production InitGlobalData, before accepting clients.
// Requires FortniteGame -> MantisRuntime dependency so its original package is loaded.
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "FFortGameplayAbilityTargetData_SingleTargetHit.h"
#include "FFortMantisTargetData.h"
#include "Serialization/BitReader.h"
#include "Serialization/BitWriter.h"

// Deliberately limited reference codec, shared by writer and reader. This is NOT a
// network GUID/name parity claim: only the vector codecs use production
// serializers; framing and hit field order are independently encoded; the three null object slots and NAME_None each use an explicit one-bit
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

// Independent recovered-original writer: NEVER call FHitResult::NetSerialize here.
// 0x144082490 uses eight flag bits and an ElementIndex byte after FaceIndex.
// The original writer always clears bit7; ForceOmitElement exercises its reader-only branch.
static bool WriteOriginalHitFixture(FNullTargetDataWriter& Ar, FHitResult& Hit, bool ForceOmitElement)
{
    const bool SamePoint = Hit.ImpactPoint == Hit.Location;
    const bool SameNormal = Hit.ImpactNormal == Hit.Normal;
    const bool InvalidItem = Hit.Item == INDEX_NONE;
    const bool InvalidFace = Hit.FaceIndex == INDEX_NONE;
    const bool NoDepth = Hit.PenetrationDepth == 0.0f;
    uint8 Flags = (Hit.bBlockingHit << 0) | (Hit.bStartPenetrating << 1)
        | (SamePoint << 2) | (SameNormal << 3) | (InvalidItem << 4)
        | (InvalidFace << 5) | (NoDepth << 6) | (ForceOmitElement << 7);
    Ar.SerializeBits(&Flags, 8);
    Ar << Hit.Time;
    bool AllSuccess = true, FieldSuccess = false;
    Hit.Location.NetSerialize(Ar, nullptr, FieldSuccess); AllSuccess &= FieldSuccess;
    Hit.Normal.NetSerialize(Ar, nullptr, FieldSuccess); AllSuccess &= FieldSuccess;
    if (!SamePoint) { Hit.ImpactPoint.NetSerialize(Ar, nullptr, FieldSuccess); AllSuccess &= FieldSuccess; }
    if (!SameNormal) { Hit.ImpactNormal.NetSerialize(Ar, nullptr, FieldSuccess); AllSuccess &= FieldSuccess; }
    Hit.TraceStart.NetSerialize(Ar, nullptr, FieldSuccess); AllSuccess &= FieldSuccess;
    Hit.TraceEnd.NetSerialize(Ar, nullptr, FieldSuccess); AllSuccess &= FieldSuccess;
    if (!NoDepth) Ar << Hit.PenetrationDepth;
    if (!InvalidItem) Ar << Hit.Item;
    FArchive& References = Ar;
    References << Hit.PhysMaterial;
    References << Hit.Actor;
    References << Hit.Component;
    References << Hit.BoneName;
    if (!InvalidFace) Ar << Hit.FaceIndex;
    if (!ForceOmitElement) Ar << Hit.ElementIndex;
    return AllSuccess && !Ar.IsError();
}

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

    // Independently write original handle framing, hit layout, and custom trailers.
    // Requires original-client codec mode and reflected uint8 FHitResult::ElementIndex.
    for (uint8 Index : {uint8(0), uint8(4), uint8(5)})
    for (int32 Variant = 0; Variant < (Index == 4 ? 1 : 3); ++Variant)
    {
        UE_LOG(LogTemp, Display, TEXT("Target-data wire fixture index=%u variant=%d"), Index, Variant);
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
        Stock.HitResult.Item = INDEX_NONE;
        Stock.HitResult.FaceIndex = INDEX_NONE;
        Stock.HitResult.ElementIndex = 17;
        if (Variant == 1)
        {
            Stock.HitResult.bStartPenetrating = true;
            Stock.HitResult.ImpactPoint = FVector(8, 24, 56);
            Stock.HitResult.ImpactNormal = FVector::ForwardVector;
            Stock.HitResult.TraceStart = FVector(1, 2, 3);
            Stock.HitResult.TraceEnd = FVector(4, 5, 6);
            Stock.HitResult.PenetrationDepth = 2.5f;
            Stock.HitResult.Item = 7;
            Stock.HitResult.FaceIndex = 11;
            // Faithful original writer sends this byte, even when value is 255.
            Stock.HitResult.ElementIndex = 255;
        }
        const bool ForceOmitElement = Variant == 2;
        if (ForceOmitElement) Stock.HitResult.ElementIndex = 255;
        if (Index == 0 || Index == 5)
            if (!WriteOriginalHitFixture(Writer, Stock.HitResult, ForceOmitElement)) return TargetDataProbeFailure(__LINE__);
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
            const FHitResult& Want = Stock.HitResult;
            if (!Hit || !Hit->bBlockingHit || Hit->Time != Want.Time || Hit->Location != Want.Location
                || Hit->bStartPenetrating != Want.bStartPenetrating
                || Hit->ImpactPoint != Want.ImpactPoint || Hit->Normal != Want.Normal
                || Hit->ImpactNormal != Want.ImpactNormal || Hit->TraceStart != Want.TraceStart
                || Hit->TraceEnd != Want.TraceEnd || Hit->PenetrationDepth != Want.PenetrationDepth
                || Hit->Item != Want.Item || Hit->FaceIndex != Want.FaceIndex
                || Hit->ElementIndex != Want.ElementIndex || !Hit->BoneName.IsNone()
                || Hit->PhysMaterial.Get() || Hit->Actor.Get() || Hit->Component.Get())
                return TargetDataProbeFailure(__LINE__);
        }
        uint32 LoadedMarker = 0;
        Reader << LoadedMarker;
        if (Reader.IsError() || LoadedMarker != Marker || Reader.GetPosBits() != Writer.GetNumBits()) return TargetDataProbeFailure(__LINE__);
    }
    return true;
}
