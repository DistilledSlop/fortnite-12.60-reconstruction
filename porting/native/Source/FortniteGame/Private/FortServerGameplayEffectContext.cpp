#include "FortServerGameplayEffectContext.h"

FGameplayEffectContext* FFortServerGameplayEffectContext::Duplicate() const
{
	FFortServerGameplayEffectContext* NewContext = new FFortServerGameplayEffectContext();
	*NewContext = *this;
	if (GetHitResult())
	{
		NewContext->AddHitResult(*GetHitResult(), true);
	}
	return NewContext;
}

// Mirrors client 0x141605cf0: stock fields first (0x14073f640), then the Fortnite extension in this order.
bool FFortServerGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	bool bBaseSuccess = true;
	FGameplayEffectContext::NetSerialize(Ar, Map, bBaseSuccess);

	// 1. One bit: the source object is of one specific class (0x142939040, not identified); if set, an FString read from
	//    that object follows and the client hands it to a lookup (0x14237ca50). stand-in: the server always writes 0, so the
	//    client receives no source-object name (it does not need one for damage or cues).
	uint8 bHasSourceName = 0;
	Ar.SerializeBits(&bHasSourceName, 1);
	if (bHasSourceName)
	{
		FString SourceName;
		Ar << SourceName;
	}

	// 2. Four single-bit flags at 0x70..0x73; the fourth gates three floats.
	auto SerializeFlag = [&Ar](bool& bFlag)
	{
		uint8 Bit = bFlag ? 1 : 0;
		Ar.SerializeBits(&Bit, 1);
		bFlag = (Bit & 1) != 0;
	};
	SerializeFlag(bFlag70);
	SerializeFlag(bFlag71);
	SerializeFlag(bFlag72);
	SerializeFlag(bFlag73);
	if (bFlag73)
	{
		Ar << KnockbackMagnitude;
		Ar << KnockbackZAngle;
		Ar << StunTime;
	}

	// 3. Two unconditional floats.
	Ar << EffectDirectionX;
	Ar << EffectDirectionY;

	// 4. One bit: ItemGuid is non-zero (computed only when saving), then the GUID.
	uint8 bHasItemGuid = Ar.IsSaving() && ItemGuid.IsValid() ? 1 : 0;
	Ar.SerializeBits(&bHasItemGuid, 1);
	if (bHasItemGuid)
	{
		Ar << ItemGuid;
	}

	// 5. Tooltip tags (0x143e93920 = FGameplayTagContainer::NetSerialize).
	bool bTagsSuccess = true;
	TooltipTags.NetSerialize(Ar, Map, bTagsSuccess);

	bOutSuccess = bBaseSuccess && bTagsSuccess;
	return true;
}

FGameplayEffectContext* UFortServerAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FFortServerGameplayEffectContext();
}
