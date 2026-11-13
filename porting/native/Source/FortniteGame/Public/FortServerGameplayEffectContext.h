#pragma once

// Server-side stand-in for the client's FFortGameplayEffectContext (reflection block size 216, NetSerialize 0x141605cf0).
// The context handle writes no struct identity (0x14073f9a0), so the vanilla client always reads the Fortnite extension
// after the stock FGameplayEffectContext fields. A server that allocates the stock context leaves the client short
// (run 29: ActiveGameplayEffects overflow, NetMulticast_InvokeGameplayCueExecuted_FromSpec "Mismatch read").
// Named FortServer* so it cannot collide with a later generated FFortGameplayEffectContext.

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "UFortAbilitySystemGlobals.h"
#include "FortServerGameplayEffectContext.generated.h"

USTRUCT()
struct FORTNITEGAME_API FFortServerGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	// Client object offsets 0x70..0x73, each written as one bit. The reflected names are bIsFatalHit, bIsCriticalHit,
	// bIsDiceCritical and bIsFullBodyHit; their mapping to the four bytes is inferred (declaration order). The fourth one
	// gates the three knockback/stun floats on the wire.
	UPROPERTY() bool bFlag70 = false;
	UPROPERTY() bool bFlag71 = false;
	UPROPERTY() bool bFlag72 = false;
	UPROPERTY() bool bFlag73 = false;
	UPROPERTY() float KnockbackMagnitude = 0.0f;  // 0x74
	UPROPERTY() float KnockbackZAngle = 0.0f;     // 0x78
	UPROPERTY() float StunTime = 0.0f;            // 0x7c
	UPROPERTY() float ChargeMagnitude = 0.0f;     // 0x80, not serialized
	UPROPERTY() float EffectDirectionX = 0.0f;    // 0x84
	UPROPERTY() float EffectDirectionY = 0.0f;    // 0x88
	UPROPERTY() FGameplayTagContainer TooltipTags; // 0x90
	UPROPERTY() int32 CartridgeID = 0;            // 0xb0, not serialized
	UPROPERTY() FGuid ItemGuid;                   // 0xb4
	UPROPERTY() float SourceLevel = 0.0f;         // 0xcc, not serialized
	UPROPERTY() float UnmodifiedDamage = 0.0f;    // 0xd0, not serialized

	virtual UScriptStruct* GetScriptStruct() const override { return StaticStruct(); }
	virtual FGameplayEffectContext* Duplicate() const override;
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
};

template<>
struct TStructOpsTypeTraits<FFortServerGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FFortServerGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};

// Allocates the Fortnite context. Selected with [/Script/GameplayAbilities.AbilitySystemGlobals]
// AbilitySystemGlobalsClassName=/Script/FortniteGame.FortServerAbilitySystemGlobals (server DefaultGame.ini).
UCLASS(config=Game)
class FORTNITEGAME_API UFortServerAbilitySystemGlobals : public UFortAbilitySystemGlobals
{
	GENERATED_BODY()

public:
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
