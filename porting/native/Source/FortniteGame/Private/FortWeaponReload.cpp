#include "FortWeaponReload.h"

#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "AFortPawn.h"
#include "AFortPlayerController.h"
#include "AFortWeapon.h"
#include "FFortBaseWeaponStats.h"
#include "UFortGameplayAbility_Reload.h"
#include "UFortInventoryOwnerInterface.h"
#include "UFortWeaponItemDefinition.h"
#include "UFortWorldItemDefinition.h"
#include "FortAthenaInventoryFlow.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortReload, Log, All);

namespace
{
	// Fraction of the reload time at which the magazine is filled (the client finishes at 1.0; the ability is ended shortly after).
	constexpr float LoadPoint = 0.9f;
	constexpr float EndPoint = 1.15f;

	TArray<TWeakObjectPtr<UAbilitySystemComponent>>& BoundSystems()
	{
		static TArray<TWeakObjectPtr<UAbilitySystemComponent>> Value;
		return Value;
	}

	TSet<TWeakObjectPtr<AFortWeapon>>& PendingWeapons()
	{
		static TSet<TWeakObjectPtr<AFortWeapon>> Value;
		return Value;
	}

	AFortPlayerController* ControllerOf(const AFortWeapon* Weapon)
	{
		const AFortPawn* Pawn = Weapon ? Cast<AFortPawn>(Weapon->GetInstigator()) : nullptr;
		return Pawn ? Cast<AFortPlayerController>(Pawn->GetController()) : nullptr;
	}

	// Mirrors the duration the client uses (AFortWeapon::NativeReload838): per-bullet reload types only take the share of the full
	// time that the missing rounds represent.
	float ReloadDuration(AFortWeapon* Weapon)
	{
		float Duration = Weapon->GetReloadTime();
		const FFortBaseWeaponStats* Stats = Weapon->NativeGetWeaponStats();
		if (Stats && static_cast<uint8>(Stats->ReloadType) != 0)
		{
			const int32 Capacity = Weapon->NativeGetBulletsPerClip14269b000();
			if (Capacity > 0 && Weapon->AmmoCount < Capacity)
			{
				Duration *= static_cast<float>(Capacity - Weapon->AmmoCount) / static_cast<float>(Capacity);
			}
		}
		return FMath::Max(Duration, 0.05f);
	}

	bool LoadMagazineImpl(AFortWeapon* Weapon)
	{
		AFortPlayerController* Controller = ControllerOf(Weapon);
		const int32 Capacity = Weapon->NativeGetBulletsPerClip14269b000();
		const int32 Before = Weapon->AmmoCount;
		if (!Controller || Capacity <= 0 || Before >= Capacity)
		{
			return false;
		}
		const bool bInfinite = FortWeaponReload::HasInfiniteAmmo(Weapon);
		const int32 Reserve = bInfinite ? MAX_int32 : Weapon->NativeGetReserveAmmo7D8();
		const int32 Take = FMath::Min(Capacity - Before, Reserve);
		if (Take <= 0)
		{
			return false;
		}
		int32 Loaded = Take;
		if (!bInfinite && Weapon->WeaponData)
		{
			Loaded = FortAthenaInventory::ConsumeAmmo(Controller, Weapon->WeaponData->GetAmmoWorldItemDefinitionNative(), Take);
		}
		if (Loaded <= 0)
		{
			return false;
		}
		Weapon->AmmoCount = Before + Loaded;
		Weapon->ForceNetUpdate();
		FortAthenaInventory::SetLoadedAmmo(Controller, Weapon->ItemEntryGuid, Weapon->AmmoCount);
		UE_LOG(LogFortReload, Display, TEXT("Combat: Reload weapon=%s magazine %d -> %d (capacity %d, took %d, infinite=%d)"),
			*GetNameSafe(Weapon), Before, Weapon->AmmoCount, Capacity, Loaded, bInfinite);
		return true;
	}

	void OnReloadActivated(UAbilitySystemComponent* ASC)
	{
		AFortPawn* Pawn = ASC ? Cast<AFortPawn>(ASC->GetAvatarActor()) : nullptr;
		AFortWeapon* Weapon = Pawn ? Pawn->CurrentWeapon : nullptr;
		UWorld* World = Pawn ? Pawn->GetWorld() : nullptr;
		if (!Weapon || !World || !Weapon->HasAuthority() || !Weapon->ReloadAbilitySpecHandle.IsValid()
			|| PendingWeapons().Contains(Weapon))
		{
			return;
		}
		const FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromHandle(Weapon->ReloadAbilitySpecHandle);
		if (!Spec || !Spec->IsActive())
		{
			return;
		}
		const FGameplayAbilitySpecHandle Handle = Weapon->ReloadAbilitySpecHandle;
		const float Duration = ReloadDuration(Weapon);
		PendingWeapons().Add(Weapon);
		TWeakObjectPtr<AFortWeapon> WeakWeapon(Weapon);
		TWeakObjectPtr<UAbilitySystemComponent> WeakASC(ASC);
		FTimerHandle LoadTimer;
		World->GetTimerManager().SetTimer(LoadTimer, [WeakWeapon, WeakASC, Handle]()
		{
			AFortWeapon* Target = WeakWeapon.Get();
			UAbilitySystemComponent* System = WeakASC.Get();
			if (!Target || !System || !IsValid(Target) || !Target->IsEquipped())
			{
				return;
			}
			const FGameplayAbilitySpec* Active = System->FindAbilitySpecFromHandle(Handle);
			// A reload the client cancelled (weapon swap, sprint, fire) has already ended its ability by now: no rounds move.
			if (Active && Active->IsActive())
			{
				LoadMagazineImpl(Target);
			}
		}, Duration * LoadPoint, false);
		FTimerHandle EndTimer;
		World->GetTimerManager().SetTimer(EndTimer, [WeakWeapon, WeakASC, Handle]()
		{
			PendingWeapons().Remove(WeakWeapon);
			if (UAbilitySystemComponent* System = WeakASC.Get())
			{
				const FGameplayAbilitySpec* Active = System->FindAbilitySpecFromHandle(Handle);
				if (Active && Active->IsActive())
				{
					// The native ability has no server-side body that ends it, and a reload that never ends blocks the next one.
					System->CancelAbilityHandle(Handle);
				}
			}
		}, Duration * EndPoint, false);
	}
}

bool FortWeaponReload::LoadMagazine(AFortWeapon* Weapon)
{
	return Weapon && Weapon->HasAuthority() && LoadMagazineImpl(Weapon);
}

void FortWeaponReload::BindAbilitySystem(UAbilitySystemComponent* ASC)
{
	if (!ASC || !ASC->IsOwnerActorAuthoritative())
	{
		return;
	}
	BoundSystems().RemoveAll([](const TWeakObjectPtr<UAbilitySystemComponent>& Entry) { return !Entry.IsValid(); });
	if (BoundSystems().Contains(ASC))
	{
		return;
	}
	BoundSystems().Add(ASC);
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC(ASC);
	ASC->AbilityActivatedCallbacks.AddLambda([WeakASC](UGameplayAbility* Ability)
	{
		if (Ability && Ability->IsA(UFortGameplayAbility_Reload::StaticClass()))
		{
			OnReloadActivated(WeakASC.Get());
		}
	});
}

bool FortWeaponReload::HasInfiniteAmmo(AFortWeapon* Weapon)
{
	if (!Weapon || !Weapon->NativeUsesAmmo820())
	{
		return true;
	}
	IFortInventoryOwnerInterface* Owner = Weapon->NativeGetInventoryOwnerInterface14269c8f0();
	return Owner && Weapon->WeaponData && Owner->NativeHasInfiniteAmmo(Weapon->WeaponData);
}

void FortWeaponReload::ConsumeShot(AFortPlayerController* Controller, AFortWeapon* Weapon, int32 Cost)
{
	if (!Weapon || Cost <= 0 || HasInfiniteAmmo(Weapon))
	{
		return;
	}
	const int32 Before = Weapon->AmmoCount;
	Weapon->AmmoCount = FMath::Max(0, Before - Cost);
	if (Weapon->AmmoCount != Before)
	{
		Weapon->ForceNetUpdate();
		FortAthenaInventory::SetLoadedAmmo(Controller, Weapon->ItemEntryGuid, Weapon->AmmoCount);
	}
}
