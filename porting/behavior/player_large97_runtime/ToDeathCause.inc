// UNCOUNTED DRAFT - 0x141a5f1c0 (869-line decompile); wrapper 0x142a8e240 steps (FGameplayTagContainer, bool) and returns EDeathCause.
// static EDeathCause AFortPlayerStateAthena::ToDeathCause(const FGameplayTagContainer& TagContainer, bool bWasDBNO)
// bWasDBNO is the second wrapper arg (read as int, tested !=0); it only feeds the DBNO-selected rows (tag slots 0x358/0x360).
// Decision order is the native order: override provider -> stage 1 rows (first match wins) -> 0xab8 gate -> stage 2 rows.
// The 44 near-identical native if-blocks are collapsed into the data tables of ToDeathCause_TagRows.inc (generated, order-preserving).
#include "Unresolved97.h"
#include "ToDeathCause_TagRows.inc"

namespace
{
	// Native tag test: slot is "valid" unless (ComparisonIndex==None(0) && Number==0).
	FORCEINLINE bool Death_SlotValid(const FRawTag& T) { return T.Number != 0 || T.ComparisonIndex != 0; }
	// Inlined scan of stage 1: GameplayTags first, then ParentTags, exact (index, number) match.
	FORCEINLINE bool Death_InlineScan(const FRawTagArray& A, const FRawTag& T)
	{
		for (int32 i = 0; i < A.Num; ++i) if (A.Data[i].ComparisonIndex == T.ComparisonIndex && A.Data[i].Number == T.Number) return true;
		return false;
	}
	FORCEINLINE const FRawTag& Death_Slot(const uint8* Table, uint32 Off) { return *reinterpret_cast<const FRawTag*>(Table + Off); }
	FORCEINLINE bool Death_Stage1Has(const FRawTagContainerView& C, const FRawTag& T)
	{
		return Death_SlotValid(T) && (Death_InlineScan(C.GameplayTags, T) || Death_InlineScan(C.ParentTags, T));
	}
	// Stage 2 / gate use the out-of-line helper 0x14192b2d0 on &C[0] then &C[0x10] (semantics not recovered).
	FORCEINLINE bool Death_Stage2Has(const FRawTagContainerView& C, const FRawTag& T)
	{
		return Death_SlotValid(T) && (Unresolved_TagArrayHasTag_14192b2d0(C.GameplayTags, T) || Unresolved_TagArrayHasTag_14192b2d0(C.ParentTags, T));
	}
}

EDeathCause AFortPlayerStateAthena::ToDeathCause(const FGameplayTagContainer& TagContainer, bool bWasDBNO)
{
	const FRawTagContainerView& C = *reinterpret_cast<const FRawTagContainerView*>(&TagContainer);

	uint8 Cause = static_cast<uint8>(EDeathCause::Unspecified); // abStackX_18[0] = 0x32
	Unresolved_DeathCauseOverrideLookup_141e5f200(Unresolved_DeathCauseOverrideProvider_141e5b2a0(), &C, bWasDBNO, &Cause);
	if (Cause != static_cast<uint8>(EDeathCause::Unspecified))
	{
		return static_cast<EDeathCause>(Cause); // override hit: native returns it without touching the ladder
	}

	// Preserve the native singleton call for each stage-1 row.
	for (const FFortToDeathCauseRow& Row : GToDeathCauseStage1)
	{
		if (!Death_Stage1Has(C, Death_Slot(Unresolved_FFortGameplayTags_Singleton_141ed1800(), Row.TagSlotOffset))) continue;
		switch (Row.Kind)
		{
		case EFortToDeathCauseRow::Fixed:        return Row.ResultA;
		case EFortToDeathCauseRow::SelectByDBNO: return bWasDBNO ? Row.ResultB : Row.ResultA; // 0x358: param*16+1 ; 0x360: -(param!=0)&0x11
		case EFortToDeathCauseRow::NestedSniper: // 0xa80 matched: then 0x638 present -> Sniper(6) else SniperNoScope(7); no further rows
			return Death_Stage1Has(C, Death_Slot(Unresolved_FFortGameplayTags_Singleton_141ed1800(), 0x638)) ? Row.ResultB : Row.ResultA;
		default: break;
		}
	}

	// Gate (0xab8): present (helper 0x14192b2d0 on either container array) => result byte 0xe.
	if (Death_Stage2Has(C, Death_Slot(Unresolved_FFortGameplayTags_Singleton_141ed1800(), GToDeathCauseGateSlot)))
	{
		return static_cast<EDeathCause>(0xe); // EDeathCause::Minigun
	}

	for (const FFortToDeathCauseRow& Row : GToDeathCauseStage2)
	{
		const bool bHit = Death_Stage2Has(C, Death_Slot(Unresolved_FFortGameplayTags_Singleton_141ed1800(), Row.TagSlotOffset));
		if (Row.Kind == EFortToDeathCauseRow::LastRow) return bHit ? Row.ResultB : Row.ResultA; // 0x448: match 0x2e (SilentSwitchingToSpectate), else 0x32 (Unspecified)
		if (bHit) return Row.ResultA;
	}
	return EDeathCause::Unspecified; // unreachable: LastRow always returns
}
