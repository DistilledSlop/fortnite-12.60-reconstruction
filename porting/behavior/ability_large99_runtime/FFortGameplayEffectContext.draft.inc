// DRAFT - UNCOMPILED, UNCOUNTED. Recovered bodies only; every unrecovered virtual stays a declaration (header).

namespace Ability99
{
	// Offset-proven raw accessor (asm offset only; the reflected 12.60 member name is NOT yet mapped by root). Declaration only.
	template<typename T> T& RawField(UObject* Object, int32 Offset);                                         // reads *(T*)((uint8*)Object + Offset)

	// 0x1410fe060 (not yet captured): lookup of an entry by FString key inside the container object at AFortPlayerController+0x1af8; yields an entry whose +0x88 is a UObject.
	void* FindHeroEntryByKey(UObject* Container, const FString& Key);                                       // 0x1410fe060 DECLARATION ONLY
	// Damage-source interface vslot 0x60 (this, ActorTags, SpecTags): implementer-specific target, not resolvable from the interface alone.
	void DamageSourceGetOwnedTags(IFortDamageSourceInterface* Source, FGameplayTagContainer& ActorTags, FGameplayTagContainer& SpecTags);
}

static AFortPlayerPawn* PlayerStatePlayerPawn(AFortPlayerState* PlayerState);                       // 0x1419b6d20 (defined below)
static UFortAccountItem* ResolveHeroAccountItem(AFortPlayerController* Controller, const FString& Key); // 0x14237ca50 (defined below)
static AActor* OwningPlayerStateOfBuilding(AActor* Building);                                            // 0x141ba8b20 (defined below)

// 0x1415e54a0 (native102-impl). Base-field initialisation at +0x08..+0x6c is the inline Engine FGameplayEffectContext ctor
// (weak ptrs -1/0, AbilityLevel=1 at +0x28, bitfield byte +0x6c cleared); Fort part:
FFortGameplayEffectContext::FFortGameplayEffectContext()
	: FGameplayEffectContext()
	, KnockbackMagnitude(0.f), KnockbackZAngle(0.f), StunTime(0.f), ChargeMagnitude(0.f), EffectDirectionX(0.f), EffectDirectionY(0.f)
	, CartridgeID(-1)
	, SourceLevel(0.f), UnmodifiedDamage(0.f)
{
	// dword at +0x70 (the four bool bytes) zeroed by the member defaults
	// TooltipTags (0x90..0xb0), ItemGuid (0xb4..0xc4) zero; DamageSourceObject = FWeakObjectPtr(nullptr) via 0x142ee3b30(+0xc4, 0).
}

// 0x14160d0f0 (asm: movss [rdi+0xcc],xmm2 ; mov [rdi+0xc4],weak). The weak-object source is Source->vslot 0x8 (interface _getUObject), checked
// against class 0x142eeab70 (name unresolved: Cast target below is a placeholder for that class) else nullptr.
void FFortGameplayEffectContext::SetDamageSource(IFortDamageSourceInterface* Source, float InSourceLevel)
{
	UObject* SourceObject = nullptr;
	if (Source != nullptr)
	{
		UObject* Candidate = Source->_getUObject();
		if (Candidate != nullptr && Candidate->GetClass()->IsChildOf(UObject::StaticClass()))   // class 0x142eeab70 == UObject
		{
			SourceObject = Candidate;
		}
	}
	DamageSourceObject = SourceObject;   // 0xc4
	SourceLevel = InSourceLevel;         // 0xcc
}

// 0x1415e9820 (native102-context): UFortAbilitySystemGlobals vtable 0x145d1ac28 slot 0x270 override of AllocGameplayEffectContext.
// alloc 0xd8 (FUN_141384ca0) then ctor 0x1415e54a0; returns null if the allocation fails.
FGameplayEffectContext* UFortAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FFortGameplayEffectContext();
}

// ---------------------------------------------------------------------------------------------------------------------------------------
// Recovered context virtuals (native103-large-deps / native103-context-deps).

// 0x1415e7cd0 destructor: frees TooltipTags arrays (0x90 data, 0xa0 parent tags), then the inline FGameplayEffectContext teardown
// (HitResult shared ptr +0x58, Actors +0x40). Equivalent to the compiler-generated destructor.
FFortGameplayEffectContext::~FFortGameplayEffectContext() = default;

// 0x1415fcb40 -> thunk 0x142788750: lazy StaticStruct cache DAT_14817ca70 = construct fn 0x142788cf0, L"FortGameplayEffectContext", size 0xd8.
UScriptStruct* FFortGameplayEffectContext::GetScriptStruct() const
{
	return FFortGameplayEffectContext::StaticStruct();
}

// 0x1415f21d0 Duplicate
FGameplayEffectContext* FFortGameplayEffectContext::Duplicate() const
{
	FFortGameplayEffectContext* NewContext = new FFortGameplayEffectContext();      // alloc 0xd8 + ctor 0x1415e54a0
	*static_cast<FGameplayEffectContext*>(NewContext) = *this;                      // 0x14077b0a0: base assignment
	NewContext->bIsFatalHit = bIsFatalHit;
	NewContext->bIsCriticalHit = bIsCriticalHit;
	NewContext->bIsDiceCritical = bIsDiceCritical;
	NewContext->bIsFullBodyHit = bIsFullBodyHit;
	NewContext->KnockbackMagnitude = KnockbackMagnitude;
	NewContext->KnockbackZAngle = KnockbackZAngle;
	NewContext->StunTime = StunTime;
	NewContext->ChargeMagnitude = ChargeMagnitude;
	NewContext->EffectDirectionX = EffectDirectionX;
	NewContext->EffectDirectionY = EffectDirectionY;
	NewContext->TooltipTags = TooltipTags;                                          // FGameplayTagContainer_CopyAssign
	NewContext->CartridgeID = CartridgeID;
	NewContext->ItemGuid = ItemGuid;
	NewContext->DamageSourceObject = DamageSourceObject;                            // raw 8 bytes at 0xc4
	NewContext->SourceLevel = SourceLevel;
	NewContext->UnmodifiedDamage = UnmodifiedDamage;
	NewContext->AddActors(Actors, false);                                           // new->vslot 0x58 with this+0x40
	if (GetHitResult() != nullptr)                                                  // this->vslot 0x78 (non-const GetHitResult)
	{
		NewContext->AddHitResult(*GetHitResult(), true);                            // new->vslot 0x60
	}
	return NewContext;
}

// 0x141605cf0 NetSerialize. Wire order is exactly the body order; CartridgeID, SourceLevel, UnmodifiedDamage, DamageSourceObject are NOT serialized.
// Archive vslot 0x158 = SerializeBits(&byte, 1); Archive+0x28 bit 1 = ArIsSaving; Archive+0x29 bit 5 handled by the inline Ar << float fast path.
bool FFortGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	FGameplayEffectContext::NetSerialize(Ar, Map, bOutSuccess);                     // 0x14073f640 (args unchanged in asm: rcx,rdx,r8,r9)

	// (1) optional hero reference carried as a string. Class 0x142939040 == UFortHero (native_class_index).
	UFortHero* Hero = Cast<UFortHero>(SourceObject.Get());                          // weak at +0x2c (Engine SourceObject)
	uint8 bHasHero = (Hero != nullptr);
	Ar.SerializeBits(&bHasHero, 1);
	if (bHasHero)
	{
		// 12.60 reflection: UFortPersistableItem::InstanceId is the FString at +0x90, inherited by UFortHero.
		FString HeroKey = (Hero != nullptr) ? Hero->InstanceId : FString();
		Ar << HeroKey;                                                              // 0x142ba63b0
		if (bHasHero && !SourceObject.IsValid())                                // 0x1406dfbc0(+0x2c) == !FWeakObjectPtr::IsValid (body proven)
		{
			// Loading: resolve through the original instigator's controller.
			if (AActor* OrigInstigator = GetOriginalInstigator())                   // vslot 0x38
			{
				if (AFortPlayerState* PlayerState = Cast<AFortPlayerState>(OrigInstigator))   // class 0x142a85850
				{
					// PlayerState+0xe0 holds the owning controller (offset from asm; AActor::Owner assumed - root to confirm)
					if (AFortPlayerController* PC = Cast<AFortPlayerController>(PlayerState->GetOwner()))   // class 0x142a3d9f0
					{
						UFortAccountItem* Resolved = ResolveHeroAccountItem(PC, HeroKey);   // 0x14237ca50 (below)
						AddSourceObject(Resolved);                                           // vslot 0x48
					}
				}
			}
		}
	}

	// (2) four bool bytes, each stored as (value & 1)
	uint8 B;
	B = bIsFatalHit;     Ar.SerializeBits(&B, 1); bIsFatalHit = (B & 1) != 0;       // +0x70
	B = bIsCriticalHit;  Ar.SerializeBits(&B, 1); bIsCriticalHit = (B & 1) != 0;    // +0x71
	B = bIsDiceCritical; Ar.SerializeBits(&B, 1); bIsDiceCritical = (B & 1) != 0;   // +0x72
	B = bIsFullBodyHit;  Ar.SerializeBits(&B, 1); bIsFullBodyHit = (B & 1) != 0;    // +0x73
	if (bIsFullBodyHit)
	{
		Ar << KnockbackMagnitude;   // 0x74
		Ar << KnockbackZAngle;      // 0x78
		Ar << StunTime;             // 0x7c
	}
	Ar << EffectDirectionX;         // 0x84 (unconditional)
	Ar << EffectDirectionY;         // 0x88

	// (3) item guid only when saving a non-zero guid; on load the bit is read
	uint8 bHasItemGuid = 0;
	if (Ar.IsSaving())
	{
		bHasItemGuid = ItemGuid.IsValid();                                          // any of the four ints (0xb4..0xc4) non-zero
	}
	Ar.SerializeBits(&bHasItemGuid, 1);
	if (bHasItemGuid)
	{
		Ar << ItemGuid;                                                             // 0x14105e0d0
	}

	TooltipTags.NetSerialize(Ar, Map, bOutSuccess);                                 // 0x143e93920 on +0x90
	return true;
}

// 0x141610690 ToString: instigator-style name of weak at +8 (Engine Instigator), else "NONE".
FString FFortGameplayEffectContext::ToString() const
{
	if (Instigator.IsValid())
	{
		return Instigator.Get()->GetFName().ToString();                             // FUN_142ce9f90 (FName::ToString)
	}
	return FString(TEXT("NONE"));
}

// 0x1415fc040 GetOwnedGameplayTags: Super, then tags from the damage-source interface; falls back to the SourceObject.
// Interface class 0x1428cead0 = IFortDamageSourceInterface; interface method at vslot 0x60 (name UNRESOLVED -> declared).
void FFortGameplayEffectContext::GetOwnedGameplayTags(FGameplayTagContainer& ActorTagContainer, FGameplayTagContainer& SpecTagContainer) const
{
	FGameplayEffectContext::GetOwnedGameplayTags(ActorTagContainer, SpecTagContainer);   // 0x14073e850 (body proven: Instigator via UGameplayTagAssetInterface 0x143e98d10 vslot 0x10, else InstigatorASC (+0x34) tag-asset subobject +0x38 vslot 0x10; ActorTagContainer only)
	IFortDamageSourceInterface* Interface = nullptr;
	if (UObject* DamageObj = DamageSourceObject.Get())
	{
		Interface = static_cast<IFortDamageSourceInterface*>(DamageObj->GetInterfaceAddress(UFortDamageSourceInterface::StaticClass()));
	}
	if (Interface == nullptr)
	{
		UObject* Source = GetSourceObject();                                          // vslot 0x50
		if (Source == nullptr) { return; }
		Interface = static_cast<IFortDamageSourceInterface*>(Source->GetInterfaceAddress(UFortDamageSourceInterface::StaticClass()));
		if (Interface == nullptr) { return; }
	}
	Ability99::DamageSourceGetOwnedTags(Interface, ActorTagContainer, SpecTagContainer);   // interface vslot 0x60 DECLARATION ONLY
}

// 0x1415fbf90 GetOriginalInstigator: weak at +8; if it is an ABuildingActor (class 0x142732ac0) and 0x141ba8b20 yields a replacement, return that.
AActor* FFortGameplayEffectContext::GetOriginalInstigator() const
{
	AActor* Result = Instigator.Get();
	if (Result != nullptr && Result->IsA<ABuildingActor>())
	{
		if (AActor* Replacement = OwningPlayerStateOfBuilding(Result))   // 0x141ba8b20 DECLARATION ONLY
		{
			return Replacement;
		}
	}
	return Result;
}

// 0x1415fbfe0 GetOriginalInstigatorAbilitySystemComponent: IAbilitySystemInterface (class 0x140756aa0 == UAbilitySystemInterface) on the original instigator, else weak at +0x34.
UAbilitySystemComponent* FFortGameplayEffectContext::GetOriginalInstigatorAbilitySystemComponent() const
{
	if (AActor* Original = GetOriginalInstigator())                                  // vslot 0x38
	{
		if (IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Original))
		{
			return ASI->GetAbilitySystemComponent();                                 // interface vslot 0x10
		}
	}
	return InstigatorAbilitySystemComponent.Get();                                   // +0x34
}

// 0x1415fb610 (new Fort virtual, slot 0xd0): original instigator as AFortPawn; else if it is an AFortPlayerState (0x142a85850) the controller's pawn via 0x1419b6d20.
AFortPawn* FFortGameplayEffectContext::FortSlot_0xD0_GetOriginalInstigatorPawn() const
{
	AActor* Original = GetOriginalInstigator();
	if (AFortPawn* Pawn = Cast<AFortPawn>(Original)) { return Pawn; }
	if (AFortPlayerState* PlayerState = Cast<AFortPlayerState>(Original))
	{
		return PlayerStatePlayerPawn(PlayerState);                                   // 0x1419b6d20
	}
	return nullptr;
}

// ---------------------------------------------------------------------------------------------------------------------------------------
// Callee bodies (native104-context-deps / callee_probe).

// 0x1419b6d20: PlayerState+0x278 (APlayerState pawn pointer position per asm) cast to AFortPlayerPawn (class 0x142a75f10).
static AFortPlayerPawn* PlayerStatePlayerPawn(AFortPlayerState* PlayerState)
{
	return Cast<AFortPlayerPawn>(PlayerState->GetPawn());
}

// 0x14237ca50: Controller+0x1af8 container -> 0x1410fe060 keyed lookup -> entry+0x88 cast to UFortAccountItem (class 0x14278d210); null if no container/entry.
static UFortAccountItem* ResolveHeroAccountItem(AFortPlayerController* Controller, const FString& Key)
{
	UObject* Container = Controller->MainMcpProfile;
	if (Container == nullptr) { return nullptr; }
	void* Entry = Ability99::FindHeroEntryByKey(Container, Key);                     // 0x1410fe060
	if (Entry == nullptr) { return nullptr; }
	return Cast<UFortAccountItem>(Ability99::RawField<UObject*>(reinterpret_cast<UObject*>(Entry), 0x88));
}

// 0x141ba8b20: owning AFortPlayerState of a building. B+0x350 is an int16 owner id (-1 => none).
// Searches GameState(World+0x130)'s player array (+0x230 TArray) for an AFortPlayerState whose int16 at +0x336 matches; if none, falls back to
// AFortGameMode (World+0x128, class 0x1428ea8f0) array at +0x2e0 matching int32 at +0x98c == (int16)id.
static AActor* OwningPlayerStateOfBuilding(AActor* Building)
{
	// Native 0x141ba8b20 is already recovered by this canonical member.
	return CastChecked<ABuildingActor>(Building)->GetOwningPlayerState();
}

// 0x1415f9260 (new Fort virtual, slot 0xc8): hit result + direction for this context.
// (HitActor = p2, FromActor = p3, OutHit = p4 (0x8c bytes, engine FHitResult layout, TraceStart +0x40 / TraceEnd +0x4c), OutDirection = p5).
// Parameter and function names are descriptive; behavior is the asm. Zero constants at 0x148218630 read as (0,0,0).
void FFortGameplayEffectContext::GetHitResultAndDirection(AActor* HitActor, AActor* FromActor, FHitResult& OutHit, FVector& OutDirection) const
{
	if (HitResult.IsValid())                                                         // this+0x50 (Engine HitResult TSharedPtr)
	{
		OutHit = *const_cast<FFortGameplayEffectContext*>(this)->GetHitResult();   // vslot 0x78
		OutDirection = (OutHit.TraceEnd - OutHit.TraceStart).GetSafeNormal();       // ==1 keep, <1e-8 -> Zero, else rsqrt x2 Newton
		return;
	}
	if (HitActor == nullptr) { return; }
	OutHit.Actor = HitActor;                                                         // weak at +0x6c (asm call keeps rdx=p2)
	OutHit.bBlockingHit = true;                                                      // byte0 |= 1
	OutHit.BoneName = NAME_None;                                                     // +0x7c..0x83 zeroed
	USceneComponent* HitRoot = HitActor->GetRootComponent();                         // HitActor+0x130
	OutHit.Component = Cast<UPrimitiveComponent>(HitRoot);                           // class 0x1447c8590, weak at +0x74
	const FVector HitLocation = HitRoot ? Ability99::RawField<FVector>(HitRoot, 0x1d0) : FVector::ZeroVector;   // root+0x1d0 (asm offset)
	OutHit.Location = HitLocation;                                                   // +0x10
	OutHit.ImpactPoint = HitLocation;                                                // +0x1c
	if (FromActor == nullptr)
	{
		OutDirection = FVector::ZeroVector;
	}
	else
	{
		USceneComponent* FromRoot = FromActor->GetRootComponent();
		const FVector FromLocation = FromRoot ? Ability99::RawField<FVector>(FromRoot, 0x1d0) : FVector::ZeroVector;
		OutDirection = (HitLocation - FromLocation).GetSafeNormal();
	}
	OutHit.Normal = -OutDirection;                                                   // +0x28 (sign bits flipped)
	OutHit.ImpactNormal = -OutDirection;                                             // +0x34
}
