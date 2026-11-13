#include "FortAthenaCombatEquip.h"

#include "AFortPawn.h"
#include "AFortPlayerState.h"
#include "AFortWeapon.h"
#include "UFortAbilitySystemComponent.h"
#include "UFortGameplayAbility.h"
#include "UFortWeaponItemDefinition.h"
#include "UFortAbilitySet.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemComponent.h"

namespace
{
	struct FRecord
	{
		TWeakObjectPtr<AFortWeapon> Weapon;
		TWeakObjectPtr<AFortPawn> Avatar;
		TWeakObjectPtr<AFortPlayerState> Owner;
		TWeakObjectPtr<UAbilitySystemComponent> ASC;
		TArray<TWeakObjectPtr<UClass>> Classes;
		TArray<int32> Slots;
		TArray<FGameplayAbilitySpecHandle> Handles;
		TWeakObjectPtr<UFortAbilitySet> Set;
		FFortAbilitySetHandle SetHandle;
		FFortAbilitySetHandle PublishedSetHandle;
		TArray<FGameplayAbilitySpecHandle> SetAbilitySlots;
		TArray<uint8> SetAbilityOwned;
		TArray<uint8> SetEffectDone;
		bool bSetPublished = false;
		uint64 LastGrantFrame = 0;
		bool bBusy = false;
		bool bRemoveRequested = false;
	};

	TArray<TSharedPtr<FRecord>>& Records() { static TArray<TSharedPtr<FRecord>> Value; return Value; }

	bool MatchesPublishedSet(const FFortAbilitySetHandle& A, const FFortAbilitySetHandle& B)
	{
		return A.TargetAbilitySystemComponent == B.TargetAbilitySystemComponent &&
			A.GrantedAbilityHandles == B.GrantedAbilityHandles && A.AppliedEffectHandles == B.AppliedEffectHandles;
	}

	void Clear(FRecord& Record)
	{
		if (UAbilitySystemComponent* ASC = Record.ASC.Get())
		{
			if (ASC->IsOwnerActorAuthoritative())
			{
				for (const FGameplayAbilitySpecHandle& Handle : Record.Handles)
					if (Handle.IsValid()) ASC->ClearAbility(Handle);
				for (const FGameplayAbilitySpecHandle& Handle : Record.SetHandle.GrantedAbilityHandles)
					if (Handle.IsValid()) ASC->ClearAbility(Handle);
				for (const FActiveGameplayEffectHandle& Handle : Record.SetHandle.AppliedEffectHandles)
					if (Handle.IsValid()) ASC->RemoveActiveGameplayEffect(Handle, -1);
			}
		}
		if (AFortWeapon* Weapon = Record.Weapon.Get())
		{
			for (const FGameplayAbilitySpecHandle& Handle : Record.Handles)
			{
				if (Weapon->PrimaryAbilitySpecHandle == Handle) Weapon->PrimaryAbilitySpecHandle = FGameplayAbilitySpecHandle();
				if (Weapon->SecondaryAbilitySpecHandle == Handle) Weapon->SecondaryAbilitySpecHandle = FGameplayAbilitySpecHandle();
				if (Weapon->ReloadAbilitySpecHandle == Handle) Weapon->ReloadAbilitySpecHandle = FGameplayAbilitySpecHandle();
				if (Weapon->ImpactAbilitySpecHandle == Handle) Weapon->ImpactAbilitySpecHandle = FGameplayAbilitySpecHandle();
				Weapon->EquippedAbilityHandles.Remove(Handle);
			}
			if (Record.bSetPublished && Weapon->EquippedAbilitySetHandles.Num() == 1 &&
				MatchesPublishedSet(Weapon->EquippedAbilitySetHandles[0], Record.PublishedSetHandle))
				Weapon->EquippedAbilitySetHandles.Reset();
			Weapon->ForceNetUpdate();
		}
		Record.Handles.Reset();
		Record.SetHandle.GrantedAbilityHandles.Reset();
		Record.SetHandle.AppliedEffectHandles.Reset();
	}
}

bool FortAthenaCombatEquip::EnsureGrants(AFortPlayerState* Owner, AFortPawn* Avatar,
	AFortWeapon* Weapon, const TArray<FGrant>& Grants, UFortAbilitySet* EquippedSet, bool bAuditFromTick)
{
	check(IsInGameThread());
	UAbilitySystemComponent* ASC = Owner ? Owner->AbilitySystemComponent : nullptr;
	if (!Owner || !Avatar || !Weapon || !Owner->HasAuthority() || !ASC ||
		ASC->OwnerActor != Owner || ASC->GetAvatarActor() != Avatar || Weapon->GetOwner() != Avatar)
		return false;
	for (const FGrant& Grant : Grants)
		if (!Grant.Ability) return false;

	Prune();
	TSharedPtr<FRecord> Record;
	for (const TSharedPtr<FRecord>& Candidate : Records())
		if (Candidate->Weapon.Get() == Weapon) { Record = Candidate; break; }
	if (Record && Record->bBusy) return false;
	if (Record)
	{
		bool bSame = Record->ASC.Get() == ASC && Record->Owner.Get() == Owner &&
			Record->Avatar.Get() == Avatar && Record->Set.Get() == EquippedSet && Record->Classes.Num() == Grants.Num() &&
			(!EquippedSet || (Record->SetAbilitySlots.Num() == EquippedSet->GameplayAbilities.Num() && Record->SetEffectDone.Num() == EquippedSet->GrantedGameplayEffects.Num()));
		for (int32 I = 0; bSame && I < Grants.Num(); ++I)
			bSame = Record->Classes[I].Get() == Grants[I].Ability.Get() && Record->Slots[I] == Grants[I].Slot;
		if (!bSame)
		{
			Record->bBusy = true;
			Clear(*Record);
			const bool bRemovedDuringCallback = Record->bRemoveRequested;
			Records().Remove(Record);
			Record.Reset();
			if (bRemovedDuringCallback) return false;
		}
	}
	if (!Record)
	{
		Record = MakeShared<FRecord>();
		if (EquippedSet && Weapon->EquippedAbilitySetHandles.Num() != 0) return false; // not our handle
		Record->Weapon = Weapon; Record->Avatar = Avatar; Record->Owner = Owner; Record->ASC = ASC; Record->Set = EquippedSet;
		if (EquippedSet)
		{
			Record->SetHandle.TargetAbilitySystemComponent = ASC;
			Record->SetAbilitySlots.SetNum(EquippedSet->GameplayAbilities.Num());
			Record->SetAbilityOwned.Init(0, EquippedSet->GameplayAbilities.Num());
			Record->SetEffectDone.Init(0, EquippedSet->GrantedGameplayEffects.Num());
		}
		for (const FGrant& Grant : Grants)
		{
			Record->Classes.Add(Grant.Ability.Get()); Record->Slots.Add(Grant.Slot);
			Record->Handles.Add(FGameplayAbilitySpecHandle());
		}
		Records().Add(Record);
	}
	Record->bBusy = true;
	for (int32 I = 0; I < Grants.Num(); ++I)
	{
		// GiveAbility can queue additions while the ASC list is locked. Audit only
		// from a later inventory tick, never from a GiveAbility callback/frame.
		if (bAuditFromTick && GFrameCounter > Record->LastGrantFrame && Record->Handles[I].IsValid())
		{
			FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Record->Handles[I]);
			if (!Spec || Spec->PendingRemove || Spec->SourceObject != Weapon ||
				!Spec->Ability || Spec->Ability->GetClass() != Grants[I].Ability.Get())
			{
				const FGameplayAbilitySpecHandle Stale = Record->Handles[I];
				if (Weapon->PrimaryAbilitySpecHandle == Stale) Weapon->PrimaryAbilitySpecHandle = FGameplayAbilitySpecHandle();
				if (Weapon->SecondaryAbilitySpecHandle == Stale) Weapon->SecondaryAbilitySpecHandle = FGameplayAbilitySpecHandle();
				if (Weapon->ReloadAbilitySpecHandle == Stale) Weapon->ReloadAbilitySpecHandle = FGameplayAbilitySpecHandle();
				if (Weapon->ImpactAbilitySpecHandle == Stale) Weapon->ImpactAbilitySpecHandle = FGameplayAbilitySpecHandle();
				Weapon->EquippedAbilityHandles.Remove(Stale);
				Record->Handles[I] = FGameplayAbilitySpecHandle();
			}
		}
		if (!Record->Handles[I].IsValid())
		{
			const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(FGameplayAbilitySpec(Grants[I].Ability, 1, INDEX_NONE, Weapon));
			Record->Handles[I] = Handle;
			Record->LastGrantFrame = GFrameCounter;
			if (!Record->Weapon.IsValid() || !Record->Avatar.IsValid() || !Record->Owner.IsValid() ||
				Record->ASC.Get() != ASC || ASC->OwnerActor != Owner || ASC->GetAvatarActor() != Avatar)
				Record->bRemoveRequested = true;
			if (Handle.IsValid() && !Record->bRemoveRequested)
			{
				switch (Grants[I].Slot)
				{
				case 0: Weapon->PrimaryAbilitySpecHandle = Handle; break;
				case 1: Weapon->SecondaryAbilitySpecHandle = Handle; break;
				case 2: Weapon->ReloadAbilitySpecHandle = Handle; break;
				case 3: Weapon->ImpactAbilitySpecHandle = Handle; break;
				default: Weapon->EquippedAbilityHandles.Add(Handle); break;
				}
				Weapon->ForceNetUpdate();
			}
			if (Record->bRemoveRequested) break;
			UE_LOG(LogTemp, Display, TEXT("Combat: Grant owner=%s weapon=%s ability=%s level=%d input=%d spec=%s"),
				*Owner->GetName(), *Weapon->GetName(), *Grants[I].Ability->GetName(), 1, INDEX_NONE, *Handle.ToString());
		}
	}
	if (EquippedSet && !Record->bRemoveRequested)
	{
		// 0x1415f3960 deduplicates existing non-pending specs by CDO and
		// source. Skipped handles belong to their original grantor.
		for (int32 I = 0; I < EquippedSet->GameplayAbilities.Num(); ++I)
		{
			UClass* Class = EquippedSet->GameplayAbilities[I].Get();
			if (!Class) { Record->SetAbilityOwned[I] = 2; continue; } // original skips null
			if (bAuditFromTick && GFrameCounter > Record->LastGrantFrame && Record->SetAbilitySlots[I].IsValid())
			{
				const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Record->SetAbilitySlots[I]);
				if (!Spec || Spec->PendingRemove || Spec->SourceObject != Weapon || !Spec->Ability || Spec->Ability->GetClass() != Class)
				{
					if (Record->SetAbilityOwned[I] == 1) Record->SetHandle.GrantedAbilityHandles.Remove(Record->SetAbilitySlots[I]);
					Record->SetAbilitySlots[I] = FGameplayAbilitySpecHandle(); Record->SetAbilityOwned[I] = 0;
				}
			}
			if (Record->SetAbilitySlots[I].IsValid()) continue;
			for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
				if (!Spec.PendingRemove && Spec.SourceObject == Weapon && Spec.Ability && Spec.Ability->GetClass() == Class)
				{
					Record->SetAbilitySlots[I] = Spec.Handle; Record->SetAbilityOwned[I] = 0; break;
				}
			if (!Record->SetAbilitySlots[I].IsValid())
			{
				const FGameplayAbilitySpecHandle Handle = ASC->GiveAbility(FGameplayAbilitySpec(EquippedSet->GameplayAbilities[I], 1, INDEX_NONE, Weapon));
				Record->LastGrantFrame = GFrameCounter;
				Record->SetAbilitySlots[I] = Handle;
				if (Handle.IsValid())
				{
					Record->SetAbilityOwned[I] = 1; Record->SetHandle.GrantedAbilityHandles.Add(Handle);
					if (Record->Owner.IsValid() && Record->Weapon.IsValid()) UE_LOG(LogTemp, Display, TEXT("Combat: SetGrant owner=%s weapon=%s ability=%s level=1 input=-1 spec=%s"),
						*Owner->GetName(), *Weapon->GetName(), *Class->GetName(), *Handle.ToString());
				}
			}
			if (!Record->Weapon.IsValid() || !Record->Avatar.IsValid() || !Record->Owner.IsValid() || ASC->OwnerActor != Owner || ASC->GetAvatarActor() != Avatar) Record->bRemoveRequested = true;
			if (Record->bRemoveRequested) break;
		}
		if (!Record->bRemoveRequested)
		{
			const FGameplayTag SetSourceTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Source.FortAbilitySet")), false);
			for (int32 I = 0; I < EquippedSet->GrantedGameplayEffects.Num() && !Record->bRemoveRequested; ++I)
			{
				const FGameplayEffectApplicationInfoHard& Info = EquippedSet->GrantedGameplayEffects[I];
				if (Record->SetEffectDone[I]) continue;
				if (!Info.GameplayEffect) { Record->SetEffectDone[I] = 1; continue; }
				if (!SetSourceTag.IsValid()) continue;
				FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
				Context.AddSourceObject(Weapon);
				FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(Info.GameplayEffect, Info.Level, Context);
				if (!Spec.IsValid()) continue;
				Spec.Data->CapturedSourceTags.GetSpecTags().AddTag(SetSourceTag);
				const FActiveGameplayEffectHandle Handle = ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), ASC);
				// Instant effects are successful without an active handle; never repeat them.
				if (Handle.WasSuccessfullyApplied()) Record->SetEffectDone[I] = 1;
				if (Handle.IsValid()) Record->SetHandle.AppliedEffectHandles.Add(Handle);
				if (Handle.WasSuccessfullyApplied() && Record->Owner.IsValid() && Record->Weapon.IsValid())
					UE_LOG(LogTemp, Display, TEXT("Combat: SetEffect owner=%s weapon=%s effect=%s level=%g handle=%s"),
						*Owner->GetName(), *Weapon->GetName(), *Info.GameplayEffect->GetName(), Info.Level, *Handle.ToString());
				if (!Record->Weapon.IsValid() || !Record->Avatar.IsValid() || !Record->Owner.IsValid() || ASC->OwnerActor != Owner || ASC->GetAvatarActor() != Avatar) Record->bRemoveRequested = true;
			}
		}
		if (!Record->bRemoveRequested)
		{
			if (!Record->bSetPublished && Weapon->EquippedAbilitySetHandles.Num() == 0)
			{
				Weapon->EquippedAbilitySetHandles.Add(Record->SetHandle);
				Record->PublishedSetHandle = Record->SetHandle;
				Record->bSetPublished = true;
			}
			else if (Record->bSetPublished && Weapon->EquippedAbilitySetHandles.Num() == 1 &&
				MatchesPublishedSet(Weapon->EquippedAbilitySetHandles[0], Record->PublishedSetHandle))
			{
				Weapon->EquippedAbilitySetHandles[0] = Record->SetHandle;
				Record->PublishedSetHandle = Record->SetHandle;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Combat: equipped ability-set handles changed externally on %s"), *Weapon->GetName());
				Record->bRemoveRequested = true;
			}
		}
	}
	if (Record->bRemoveRequested)
	{
		Clear(*Record);
		Records().Remove(Record);
		return false;
	}
	bool bComplete = true;
	for (const FGameplayAbilitySpecHandle& Handle : Record->Handles) bComplete &= Handle.IsValid();
	if (EquippedSet)
	{
		bComplete &= Record->bSetPublished;
		for (int32 I = 0; I < Record->SetAbilitySlots.Num(); ++I) bComplete &= Record->SetAbilitySlots[I].IsValid() || Record->SetAbilityOwned[I] == 2;
		for (uint8 Done : Record->SetEffectDone) bComplete &= Done != 0;
	}
	Record->bBusy = false;
	return bComplete;
}

bool FortAthenaCombatEquip::OnUnequip(AFortWeapon* Weapon)
{
	check(IsInGameThread());
	if (!Weapon) return true;
	// Athena 0x1426aafb0. A configured but not yet resolved set has no
	// reliable handle state; leave the current weapon in place and retry.
	if (Weapon->WeaponData && !Weapon->WeaponData->EquippedAbilitySet.IsNull() && Weapon->EquippedAbilitySetHandles.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Combat: unresolved equipped ability set lifetime on %s"), *Weapon->GetName());
		return false;
	}
	if (!Weapon->bRemoveAbilitiesWhenRemovedFromInventory || Weapon->EquippedAbilityHandles.Num() > 0 || Weapon->EquippedAbilitySetHandles.Num() > 0)
		RemoveWeapon(Weapon);
	return true;
}

void FortAthenaCombatEquip::RemoveWeapon(AFortWeapon* Weapon)
{
	check(IsInGameThread());
	// Iterate a snapshot: cancellation may re-enter inventory code.
	const TArray<TSharedPtr<FRecord>> Snapshot = Records();
	for (const TSharedPtr<FRecord>& Record : Snapshot)
		if (Record->Weapon.Get() == Weapon)
		{
			if (Record->bBusy) { Record->bRemoveRequested = true; continue; }
			Record->bBusy = true;
			Clear(*Record);
			Records().Remove(Record);
		}
}

void FortAthenaCombatEquip::Prune()
{
	check(IsInGameThread());
	const TArray<TSharedPtr<FRecord>> Snapshot = Records();
	for (const TSharedPtr<FRecord>& Record : Snapshot)
	{
		UAbilitySystemComponent* ASC = Record->ASC.Get();
		if (!Record->bBusy && (!Record->Weapon.IsValid() || !Record->Avatar.IsValid() || !Record->Owner.IsValid() || !ASC ||
			ASC->OwnerActor != Record->Owner.Get() || ASC->GetAvatarActor() != Record->Avatar.Get()))
		{
			Record->bBusy = true;
			Clear(*Record);
			Records().Remove(Record);
		}
	}
}
