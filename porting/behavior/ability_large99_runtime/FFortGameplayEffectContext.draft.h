// DRAFT - UNCOMPILED, UNCOUNTED. Not a generated header; NOT for canonical promotion until root generator support exists.
// Source of truth: porting/generated/reflection/struct_blocks.json record "FortGameplayEffectContext" (block 0x14793e900, size 0xd8 align 8,
//   super 0x14077df70 = FGameplayEffectContext, flags Native|HasInstancedReference, struct_ops vtable 0x14629ed10: HasNetSerializer, HasCopy, HasDestructor),
//   ctor 0x1415e54a0 (native102-impl), allocator 0x1415e9820 (native102-context), primary vtable 0x145d19518 (read from the runtime image).
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "UFortDamageSourceInterface.h"

#include "FFortGameplayEffectContext.generated.h"

USTRUCT()
struct FORTNITEGAME_API FFortGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

	// 12.60 FBoolPropertyParams setters prove the names and offsets independently of table order.
	UPROPERTY() bool bIsFatalHit = false;      // 0x70; setter 0x140746250
	UPROPERTY() bool bIsCriticalHit = false;   // 0x71; setter 0x1410e8e70
	UPROPERTY() bool bIsDiceCritical = false;  // 0x72; setter 0x14153e980
	UPROPERTY() bool bIsFullBodyHit = false;   // 0x73; setter 0x14153e970

	// Members are listed in declaration order (the table is reverse order). Offsets are from the metadata.
	UPROPERTY() float KnockbackMagnitude = 0.f;   // 0x74
	UPROPERTY() float KnockbackZAngle = 0.f;      // 0x78
	UPROPERTY() float StunTime = 0.f;             // 0x7c
	UPROPERTY() float ChargeMagnitude = 0.f;      // 0x80
	UPROPERTY() float EffectDirectionX = 0.f;     // 0x84
	UPROPERTY() float EffectDirectionY = 0.f;     // 0x88
	UPROPERTY() FGameplayTagContainer TooltipTags;// 0x90 (struct 0x143e9a300, 32 bytes to 0xb0)
	UPROPERTY() int32 CartridgeID = -1;           // 0xb0 (ctor writes 0xffffffff at +0xb0)
	UPROPERTY() FGuid ItemGuid;                   // 0xb4 (struct 0x142eeb130; 16 bytes to 0xc4; ctor zeroes 0xb4/0xbc)
	UPROPERTY() TWeakObjectPtr<UObject> DamageSourceObject; // 0xc4 (metadata class getter 0x142eea520 -> jmp 0x1406966e0, treated as UObject; helper cast class 0x142eeab70 == UObject proven by native_class_index)
	UPROPERTY() float SourceLevel = 0.f;          // 0xcc
	UPROPERTY() float UnmodifiedDamage = 0.f;     // 0xd0


	FFortGameplayEffectContext();                 // 0x1415e54a0

	// Recovered effect of 0x14160d0f0 (asm-proven): DamageSourceObject <- weak(Source's UObject), SourceLevel <- InSourceLevel.
	void SetDamageSource(IFortDamageSourceInterface* Source, float InSourceLevel);

	// Primary vtable 0x145d19518, slot = offset/8, Engine 4.23 order (GameplayEffectTypes.h). Overrides = non-Engine target addresses:
	virtual ~FFortGameplayEffectContext();                                                   // 0x00 0x1415e7cd0
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& ActorTagContainer, FGameplayTagContainer& SpecTagContainer) const override; // 0x08 0x1415fc040
	virtual AActor* GetOriginalInstigator() const override;                                  // 0x38 0x1415fbf90
	virtual UAbilitySystemComponent* GetOriginalInstigatorAbilitySystemComponent() const override; // 0x40 0x1415fbfe0
	virtual FString ToString() const override;                                               // 0x98 0x141610690
	virtual UScriptStruct* GetScriptStruct() const override;                                 // 0xa0 0x1415fcb40
	virtual FGameplayEffectContext* Duplicate() const override;                              // 0xa8 0x1415f21d0
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override; // 0xc0 0x141605cf0
	// New Fort virtuals (no Engine counterpart). Names UNRESOLVED; declared by slot only:
	virtual AFortPawn* FortSlot_0xD0_GetOriginalInstigatorPawn() const;                       // 0xd0 0x1415fb610 (recovered below)
	virtual void GetHitResultAndDirection(AActor* HitActor, AActor* FromActor, FHitResult& OutHit, FVector& OutDirection) const;   // 0xc8 0x1415f9260 (recovered in .inc; name descriptive)
	// 0xd8 0x1415e7d70 is NOT a context virtual: it is the deleting dtor of another type (vtable 0x145d195f0), i.e. the next vtable entry.
	// Inherited unchanged (Engine targets): 0x10 AddInstigator, 0x18 SetAbility, ..., 0xb0 IsLocallyControlled 0x14073ef40, 0xb8 IsLocallyControlledPlayer 0x14073efe0.
};

template<> struct TStructOpsTypeTraits<FFortGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FFortGameplayEffectContext>
{
	enum { WithNetSerializer = true, WithCopy = true };   // struct_ops 0x14629ed10 traits; HasIdentical false
};

static_assert(sizeof(FFortGameplayEffectContext) == 0xd8, "metadata size 216");
static_assert(STRUCT_OFFSET(FFortGameplayEffectContext, bIsFatalHit) == 0x70, "bool bytes 0x70..0x73");
static_assert(STRUCT_OFFSET(FFortGameplayEffectContext, bIsFullBodyHit) == 0x73, "bool bytes");
static_assert(STRUCT_OFFSET(FFortGameplayEffectContext, KnockbackMagnitude) == 0x74, "base must end at 0x70 with bitfield byte at 0x70");
static_assert(STRUCT_OFFSET(FFortGameplayEffectContext, DamageSourceObject) == 0xc4, "metadata");
static_assert(STRUCT_OFFSET(FFortGameplayEffectContext, SourceLevel) == 0xcc, "metadata");
