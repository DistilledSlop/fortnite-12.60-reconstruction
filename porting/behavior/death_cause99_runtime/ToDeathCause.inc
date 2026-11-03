// UNCOUNTED candidate: 0x141a5f1c0. Provider below remains unimplemented.
#include "FortNativeGameplayTagsRuntime.h"
#include "FortAssets.h"
#include "FortDelayedObjectLoader.h"
#include "UFortGameData.h"
#include "UFortDeathCauseFromTagMapping.h"

// Exact open boundary: 0x141e5b2a0 -> 0x141df62a0 -> 0x141df7530.
// Must retain cache-miss, load, diagnostic, and failure behavior of 0x141df76b0.
UFortGameData* Unresolved_CommonGameDataProvider_141e5b2a0();

enum class EFortToDeathCauseRow : uint8 { Fixed, SelectByDBNO, NestedSniper, LastRow };
struct FFortToDeathCauseRow
{
    uint32 TagSlotOffset;
    EFortToDeathCauseRow Kind;
    EDeathCause ResultA;
    EDeathCause ResultB;
};
#include "ToDeathCause_TagRows.inc"
#include "OverrideClassification.inc"

static UFortDeathCauseFromTagMapping* ResolveDeathCauseMapping(UFortGameData& GameData)
{
    auto& Mapping = GameData.FortDeathCauseFromTagMapping;
    if (Mapping.IsNull())
        return nullptr;
    UFortDeathCauseFromTagMapping* Result = Mapping.Get();
    if (!Result)
        Result = Cast<UFortDeathCauseFromTagMapping>(FortLoadDelayedObject(Mapping.ToSoftObjectPath(), true));
    FFortAssets::Get().TrackAsset(Result);
    return Result;
}

EDeathCause AFortPlayerStateAthena::ToDeathCause(const FGameplayTagContainer& TagContainer, bool bWasDBNO)
{
    EDeathCause Cause = EDeathCause::Unspecified;
    UFortGameData* GameData = Unresolved_CommonGameDataProvider_141e5b2a0();
    // Native dereferences the provider without a null guard.
    if (UFortDeathCauseFromTagMapping* Mapping = ResolveDeathCauseMapping(*GameData))
        ClassifyDeathCauseOverride(*Mapping, TagContainer, bWasDBNO, Cause);
    if (Cause != EDeathCause::Unspecified)
        return Cause;

    for (const FFortToDeathCauseRow& Row : GToDeathCauseStage1)
    {
        if (!TagContainer.HasTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(Row.TagSlotOffset)))
            continue;
        switch (Row.Kind)
        {
        case EFortToDeathCauseRow::Fixed: return Row.ResultA;
        case EFortToDeathCauseRow::SelectByDBNO: return bWasDBNO ? Row.ResultB : Row.ResultA;
        case EFortToDeathCauseRow::NestedSniper:
            return TagContainer.HasTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x638)) ? Row.ResultB : Row.ResultA;
        default: break;
        }
    }
    if (TagContainer.HasTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(GToDeathCauseGateSlot)))
        return EDeathCause::Minigun;
    for (const FFortToDeathCauseRow& Row : GToDeathCauseStage2)
    {
        const bool bHit = TagContainer.HasTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(Row.TagSlotOffset));
        if (Row.Kind == EFortToDeathCauseRow::LastRow)
            return bHit ? Row.ResultB : Row.ResultA;
        if (bHit)
            return Row.ResultA;
    }
    return EDeathCause::Unspecified;
}
