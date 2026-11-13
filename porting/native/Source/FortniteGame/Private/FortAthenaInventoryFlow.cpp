// Server-side player inventory for the original 12.60 client (WP D4, G8 first slice). See the header for scope.
//
// Evidence summary (labels: binary / data / source / inferred / stand-in):
// - binary 0x142372de0 (client ClientRestart): on a client, restart waits until controller WorldInventory (+0x1738) and
//   ClientQuickBars (+0x14d0) are non-null. ClientQuickBars is created by the client itself (controller virtual 0x1423f20a0,
//   run from OnReadyToStartMatch), never replicated. The server only has to supply a replicated WorldInventory (done by
//   EnsureWorldInventory in FortniteAthenaGameplay.cpp, reused here, never replaced).
// - source behavior/AFortInventory/NativeUpdateEntry.inc, behavior/UFortWorldItem/*.inc: an item owns a non-replicated
//   FFortItemEntry (bIsReplicatedCopy=false); AFortInventory::Inventory.ReplicatedEntries holds the replicated copy
//   (bIsReplicatedCopy=true), indexed by guid in Inventory.NativeEntriesByGuid (raw pointers into the array).
// - binary 0x1423150b0 (AFortPawn::EquipWeaponDefinition): reuse a weapon from CurrentWeaponList whose ItemEntryGuid (+0x8c0)
//   matches, else spawn WeaponData->WeaponActorClass owned by the pawn, set WeaponData/ItemEntryGuid, push it to the front of
//   CurrentWeaponList and make it the current weapon. EquipWeaponDefinition itself is not emitted in the generated headers.
// - data (C card): the native game mode has no Blueprint defaults, so StartingItems may be empty; the 12.60 list is
//   Wall, Floor, Stair_W, RoofS (1 each), EditTool (1), WoodItemData (0).

#include "FortAthenaInventoryFlow.h"
#include "FortAthenaCombatEquip.h"
#include "EngineUtils.h"
#include "UObject/ScriptMacros.h"

#include "AFortGameModeAthena.h"
#include "AFortInventory.h"
#include "AFortPawn.h"
#include "AFortPlayerController.h"
#include "AFortPlayerControllerAthena.h"
#include "AFortPlayerState.h"
#include "AFortWeapon.h"
#include "FFortAthenaLoadout.h"
#include "FFortItemEntry.h"
#include "FItemAndCount.h"
#include "UAthenaPickaxeItemDefinition.h"
#include "UFortAbilitySystemComponent.h"
#include "UFortAbilitySet.h"
#include "FortAssets.h"
#include "FortDelayedObjectLoader.h"
#include "UFortGameplayAbility.h"
#include "UFortItemDefinition.h"
#include "UFortWeaponItemDefinition.h"
#include "UFortWeaponMeleeItemDefinition.h"
#include "UFortWorldItem.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "GameplayAbilitySpec.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortInventoryFlow, Log, All);

namespace
{
	// data: /Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01 (extracted content).
	const TCHAR* const GDefaultPickaxePath = TEXT("/Game/Athena/Items/Weapons/WID_Harvest_Pickaxe_Athena_C_T01.WID_Harvest_Pickaxe_Athena_C_T01");

	struct FDefaultStartItem
	{
		const TCHAR* Path;
		int32 Count;
	};

	// data: Athena_GameMode StartingItems (C card section 1); asset paths from the extracted content.
	const FDefaultStartItem GDefaultStartingItems[] = {
		{ TEXT("/Game/Items/Weapons/BuildingTools/BuildingItemData_Wall.BuildingItemData_Wall"), 1 },
		{ TEXT("/Game/Items/Weapons/BuildingTools/BuildingItemData_Floor.BuildingItemData_Floor"), 1 },
		{ TEXT("/Game/Items/Weapons/BuildingTools/BuildingItemData_Stair_W.BuildingItemData_Stair_W"), 1 },
		{ TEXT("/Game/Items/Weapons/BuildingTools/BuildingItemData_RoofS.BuildingItemData_RoofS"), 1 },
		{ TEXT("/Game/Items/Weapons/BuildingTools/EditTool.EditTool"), 1 },
		{ TEXT("/Game/Items/ResourcePickups/WoodItemData.WoodItemData"), 0 },
	};

	// Guid of the pickaxe item per controller, so a later pawn (respawn) can re-equip it without a second grant.
	TMap<TWeakObjectPtr<const AController>, FGuid>& PickaxeGuids()
	{
		static TMap<TWeakObjectPtr<const AController>, FGuid> Map;
		return Map;
	}

	// The index holds raw pointers into ReplicatedEntries, so it must be rebuilt after any add or remove.
	void RebuildGuidIndex(AFortInventory* Inventory)
	{
		Inventory->Inventory.NativeEntriesByGuid.Reset();
		for (FFortItemEntry& Entry : Inventory->Inventory.ReplicatedEntries)
		{
			Inventory->Inventory.NativeEntriesByGuid.Add(Entry.ItemGuid, &Entry);
		}
	}

	FFortItemEntry* FindReplicatedEntry(AFortInventory* Inventory, const FGuid& Guid)
	{
		FFortItemEntry** Found = Inventory->Inventory.NativeEntriesByGuid.Find(Guid);
		return Found ? *Found : nullptr;
	}

	UFortWorldItem* FindItemByGuid(AFortInventory* Inventory, const FGuid& Guid)
	{
		for (UFortWorldItem* Item : Inventory->Inventory.ItemInstances)
		{
			if (Item && Item->ItemEntry.ItemGuid == Guid)
			{
				return Item;
			}
		}
		return nullptr;
	}

	UFortItemDefinition* LoadDefinition(const TCHAR* Path)
	{
		UFortItemDefinition* Definition = LoadObject<UFortItemDefinition>(nullptr, Path);
		if (!Definition)
		{
			UE_LOG(LogTemp, Warning, TEXT("Inventory: item definition %s could not be loaded (content not mounted on the server?)"), Path);
		}
		return Definition;
	}

	// The player's equipped pickaxe: loadout cosmetic -> its melee WeaponDefinition (binary 0x1423150b0 swaps the harvest
	// tool's data from the controller's pickaxe the same way); otherwise the default Athena pickaxe.
	UFortItemDefinition* ResolvePickaxe(AFortPlayerController* Controller, FString& OutSource)
	{
		const UAthenaPickaxeItemDefinition* Cosmetic = Controller->CosmeticLoadoutPC.Pickaxe;
		if (Cosmetic && Cosmetic->WeaponDefinition)
		{
			OutSource = FString::Printf(TEXT("loadout %s"), *Cosmetic->GetName());
			return Cosmetic->WeaponDefinition;
		}
		OutSource = TEXT("default (no pickaxe in CosmeticLoadoutPC; stand-in until the backend loadout is read)");
		return LoadDefinition(GDefaultPickaxePath);
	}

	void GrantWeaponAbilities(AFortPlayerController* Controller, AFortPawn* Pawn, AFortWeapon* Weapon, UFortWeaponItemDefinition* Definition, bool bAuditFromTick)
	{
		if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteInventoryAbilities"))) return;
		AFortPlayerState* PlayerState = Controller ? Cast<AFortPlayerState>(Controller->PlayerState) : nullptr;
		if (!PlayerState || !Pawn || !Weapon || !Definition) return;
		UFortAbilitySet* EquippedSet = nullptr;
		if (!Definition->EquippedAbilitySet.IsNull())
		{
			EquippedSet = Definition->EquippedAbilitySet.Get();
			if (!EquippedSet) EquippedSet = Cast<UFortAbilitySet>(FortLoadDelayedObject(Definition->EquippedAbilitySet.ToSoftObjectPath(), true));
			FFortAssets::Get().TrackAsset(EquippedSet);
			if (!EquippedSet) return; // retry after the soft asset becomes loadable
		}
		TArray<FortAthenaCombatEquip::FGrant> Grants;
		auto Add = [&Grants](const TSubclassOf<UFortGameplayAbility>& Ability, int32 Slot, bool bConfigured) -> bool
		{
			if (!Ability) return !bConfigured;
			FortAthenaCombatEquip::FGrant Grant;
			Grant.Ability = Ability; Grant.Slot = Slot; Grants.Add(Grant); return true;
		};
		bool bResolved = true;
		bResolved &= Add(Definition->GetPrimaryFireAbility(), 0, !Definition->PrimaryFireAbility.IsNull());
		bResolved &= Add(Definition->GetSecondaryFireAbility(), 1, !Definition->SecondaryFireAbility.IsNull());
		bResolved &= Add(Definition->GetReloadAbility(), 2, !Definition->ReloadAbility.IsNull());
		bResolved &= Add(Definition->GetOnHitAbility(), 3, !Definition->OnHitAbility.IsNull());
		const auto Equipped = Definition->GetEquippedAbilities();
		int32 Expected = 0;
		for (const auto& SoftClass : Definition->EquippedAbilities) if (!SoftClass.IsNull()) ++Expected;
		bResolved &= Equipped.Num() == Expected;
		for (int32 I = 0; I < Equipped.Num(); ++I) bResolved &= Add(Equipped[I], I + 4, true);
		if (bResolved) FortAthenaCombatEquip::EnsureGrants(PlayerState, Pawn, Weapon, Grants, EquippedSet, bAuditFromTick);
	}

	void SetCurrentWeapon(AFortPawn* Pawn, AFortWeapon* Weapon)
	{
		if (Pawn->CurrentWeapon == Weapon)
		{
			return;
		}
		AFortWeapon* OldWeapon = Pawn->CurrentWeapon;
		if (FParse::Param(FCommandLine::Get(), TEXT("FortniteInventoryAbilities")) &&
			!FortAthenaCombatEquip::OnUnequip(OldWeapon)) return;
		// ClearAbility can re-enter inventory code; do not overwrite its equip.
		if (Pawn->CurrentWeapon != OldWeapon) return;
		// stand-in: the original goes through pawn virtual +0xc58 / 0x142a16c90 (holster the old weapon, equip animation).
		// Here only the replicated pointer changes; the client's OnRep_CurrentWeapon does the visible swap.
		Pawn->PreviousWeapon = Pawn->CurrentWeapon;
		Pawn->CurrentWeapon = Weapon;
		Pawn->ForceNetUpdate();
		// binary 0x142a16c90 is the client-RPC thunk of AFortPawn::ClientInternalEquipWeapon: the original server tells the
		// owning client to run the equip (animation set, build preview, input) in addition to replicating CurrentWeapon.
		// Sent from FortAthenaInventory::Tick once the pawn walks: sending it during warmup/skydive coincided with the client
		// leaving the match 2 s after landing (run 25; that run also had the landscape checksum drop, which may have been the cause).
	}

	// Last weapon each pawn's client was told to equip (see Tick).
	TMap<TWeakObjectPtr<AFortPawn>, TWeakObjectPtr<AFortWeapon>>& ClientEquipped()
	{
		static TMap<TWeakObjectPtr<AFortPawn>, TWeakObjectPtr<AFortWeapon>> Map;
		return Map;
	}

	AFortWeapon* EquipItem(AFortPlayerController* Controller, AFortPawn* Pawn, UFortWorldItem* Item)
	{
		UFortWeaponItemDefinition* Definition = Cast<UFortWeaponItemDefinition>(Item->ItemEntry.ItemDefinition);
		if (!Definition)
		{
			// Gadgets (AthenaGadget slots), consumables and resources are not weapon definitions: not handled in this slice.
			UE_LOG(LogTemp, Display, TEXT("Inventory: execute %s ignored: %s is not a weapon/building/edit-tool definition"),
				*Item->ItemEntry.ItemGuid.ToString(), Item->ItemEntry.ItemDefinition ? *Item->ItemEntry.ItemDefinition->GetName() : TEXT("null"));
			return nullptr;
		}
		const FGuid Guid = Item->ItemEntry.ItemGuid;

		for (AFortWeapon* Existing : Pawn->CurrentWeaponList)
		{
			if (Existing && !Existing->IsPendingKill() && Existing->ItemEntryGuid == Guid)
			{
				SetCurrentWeapon(Pawn, Existing);
				if (Pawn->CurrentWeapon == Existing) GrantWeaponAbilities(Controller, Pawn, Existing, Definition, false);
				UE_LOG(LogTemp, Display, TEXT("Inventory: re-equipped %s (%s) on %s"), *Definition->GetName(), *Guid.ToString(), *Pawn->GetName());
				return Existing;
			}
		}

		UClass* WeaponClass = Definition->GetWeaponActorClass().Get();
		if (!WeaponClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Inventory: %s has no weapon actor class; cannot equip"), *Definition->GetName());
			return nullptr;
		}
		FActorSpawnParameters Params;
		Params.Owner = Pawn;
		Params.Instigator = Pawn;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AFortWeapon* Weapon = Pawn->GetWorld()->SpawnActor<AFortWeapon>(WeaponClass, Pawn->GetActorTransform(), Params);
		if (!Weapon)
		{
			// binary 0x1423150b0 logs "EquipWeaponData - Couldn't spawn weapon %s" here.
			UE_LOG(LogTemp, Warning, TEXT("Inventory: EquipWeaponData - couldn't spawn weapon %s (%s)"), *Definition->GetName(), *WeaponClass->GetName());
			return nullptr;
		}
		Weapon->SetOwner(Pawn);
		// stand-in for the AFortWeapon constructor defaults: our generated class has no constructor, so a Blueprint weapon
		// spawned here did not replicate at all (run 27: no actor channel for DefaultBuildingTool_C / the pickaxe, nothing
		// in the client's hands, no build preview).
		Weapon->SetReplicates(true);
		Weapon->bNetUseOwnerRelevancy = true;
		Weapon->WeaponData = Definition;
		Weapon->ItemEntryGuid = Guid;
		Weapon->WeaponLevel = Item->ItemEntry.Level;
		Pawn->CurrentWeaponList.Insert(Weapon, 0);
		SetCurrentWeapon(Pawn, Weapon);
		if (Pawn->CurrentWeapon == Weapon) GrantWeaponAbilities(Controller, Pawn, Weapon, Definition, false);
		UE_LOG(LogTemp, Display, TEXT("Inventory: equipped %s (%s) as %s on %s"), *Definition->GetName(), *Guid.ToString(), *WeaponClass->GetName(), *Pawn->GetName());
		return Weapon;
	}

	AFortPawn* GetControllerPawn(AFortPlayerController* Controller)
	{
		return Controller ? Cast<AFortPawn>(Controller->GetPawn()) : nullptr;
	}
}

bool FortAthenaInventory::IsEnabled()
{
	static const bool bEnabled = FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
	return bEnabled;
}

namespace
{
	// Diagnostic (-FortniteGcProbe): run a full garbage collection before and after every grant and log the stage, so a corrupt
	// object reference (run 33: "Invalid object in GC ... FortInventory ... HeroType") is caught at the step that created it.
	// Logs the 8 bytes at Inventory+128 (the slot GC reported) after a step, to find the write that corrupts it.
	static void SnapInv(AFortInventory* Inv, const TCHAR* Step)
	{
		if (!Inv || !FParse::Param(FCommandLine::Get(), TEXT("FortniteGcProbe"))) { return; }
		uint64 V = 0;
		FMemory::Memcpy(&V, reinterpret_cast<const uint8*>(Inv) + 128, sizeof(V));
		UE_LOG(LogTemp, Display, TEXT("Inventory: GcProbe snap %s [+128]=0x%016llx"), Step, V);
	}

	// Logs every aligned 8-byte slot in [Base, Base+Size) that holds a value shaped like a code pointer in this binary
	// (upper half zero, 0x10000..0x20000000), which GC would read as an object reference ("Invalid object in GC").
	static void ScanSlots(const void* Base, int32 Size, const TCHAR* Area, const TCHAR* Step)
	{
		if (!Base || !FParse::Param(FCommandLine::Get(), TEXT("FortniteGcProbe"))) { return; }
		const uint8* B = reinterpret_cast<const uint8*>(Base);
		for (int32 Off = 0; Off + 8 <= Size; Off += 8)
		{
			uint64 V = 0;
			FMemory::Memcpy(&V, B + Off, sizeof(V));
			if (V > 0x10000ull && V < 0x20000000ull)
			{
				UE_LOG(LogTemp, Display, TEXT("Inventory: GcProbe scan %s %s +%d = 0x%llx"), Step, Area, Off, V);
			}
		}
	}

	static void ScanInventory(AFortInventory* Inv, UFortWorldItem* Item, const TCHAR* Step)
	{
		if (!Inv) { return; }
		ScanSlots(Inv, (int32)sizeof(AFortInventory), TEXT("actor"), Step);
		if (Inv->Inventory.ReplicatedEntries.Num() > 0)
		{
			ScanSlots(Inv->Inventory.ReplicatedEntries.GetData(), Inv->Inventory.ReplicatedEntries.Num() * (int32)sizeof(FFortItemEntry), TEXT("entries"), Step);
		}
		if (Inv->Inventory.ItemInstances.Num() > 0)
		{
			ScanSlots(Inv->Inventory.ItemInstances.GetData(), Inv->Inventory.ItemInstances.Num() * 8, TEXT("instances"), Step);
		}
		if (Item) { ScanSlots(Item, (int32)sizeof(UFortWorldItem), TEXT("item"), Step); }
	}

	struct FGcProbeScope
	{
		FString Label;
		static bool Enabled()
		{
			static const bool bOn = FParse::Param(FCommandLine::Get(), TEXT("FortniteGcProbe"));
			return bOn;
		}
		static void DumpLayoutOnce()
		{
			static bool bDone = false;
			if (bDone) { return; }
			bDone = true;
			UE_LOG(LogTemp, Display, TEXT("Inventory: GcProbe layout sizeof AFortInventory=%d reflected=%d FFortItemList=%d/%d FFortItemEntry=%d/%d UFortWorldItem=%d/%d FFastArraySerializer=%d"),
				(int32)sizeof(AFortInventory), AFortInventory::StaticClass()->GetStructureSize(),
				(int32)sizeof(FFortItemList), FFortItemList::StaticStruct()->GetStructureSize(),
				(int32)sizeof(FFortItemEntry), FFortItemEntry::StaticStruct()->GetStructureSize(),
				(int32)sizeof(UFortWorldItem), UFortWorldItem::StaticClass()->GetStructureSize(),
				(int32)sizeof(FFastArraySerializer));
			UE_LOG(LogTemp, Display, TEXT("Inventory: GcProbe offsetof Inventory=%d ReplayPawn=%d PendingInstances=%d RecentlyAdded=%d"),
				(int32)STRUCT_OFFSET(AFortInventory, Inventory), (int32)STRUCT_OFFSET(AFortInventory, ReplayPawn),
				(int32)STRUCT_OFFSET(AFortInventory, PendingInstances), (int32)STRUCT_OFFSET(AFortInventory, RecentlyAdded));
			for (TFieldIterator<FProperty> It(AFortInventory::StaticClass()); It; ++It)
			{
				if (It->GetOffset_ForGC() >= 96 && It->GetOffset_ForGC() <= 200)
				{
					UE_LOG(LogTemp, Display, TEXT("Inventory: GcProbe prop %s %s offset=%d size=%d"), *It->GetOwnerClass()->GetName(), *It->GetName(), It->GetOffset_ForGC(), It->GetSize());
				}
			}
			for (TFieldIterator<FProperty> It(FFortItemEntry::StaticStruct()); It; ++It)
			{
				UE_LOG(LogTemp, Display, TEXT("Inventory: GcProbe entry prop %s offset=%d size=%d"), *It->GetName(), It->GetOffset_ForGC(), It->GetSize());
			}
		}
		explicit FGcProbeScope(const FString& InLabel) : Label(InLabel)
		{
			if (Enabled())
			{
				DumpLayoutOnce();
				UE_LOG(LogTemp, Display, TEXT("Inventory: GcProbe before %s"), *Label);
				CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
			}
		}
		~FGcProbeScope()
		{
			if (Enabled())
			{
				UE_LOG(LogTemp, Display, TEXT("Inventory: GcProbe after %s"), *Label);
				CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
				UE_LOG(LogTemp, Display, TEXT("Inventory: GcProbe clean after %s"), *Label);
			}
		}
	};
}

UFortWorldItem* FortAthenaInventory::GiveItem(AFortPlayerController* Controller, UFortItemDefinition* Definition, int32 Count, int32 Level, int32 LoadedAmmo)
{
	if (!IsEnabled())
	{
		return nullptr;
	}
	FGcProbeScope GcProbe(Definition ? Definition->GetName() : FString(TEXT("null")));
	AFortInventory* Inventory = Controller ? Controller->WorldInventory : nullptr;
	if (!Inventory || !Definition || Count < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory: GiveItem rejected (inventory=%d definition=%s count=%d)"),
			Inventory ? 1 : 0, Definition ? *Definition->GetName() : TEXT("null"), Count);
		return nullptr;
	}

	ScanInventory(Inventory, nullptr, TEXT("before"));
	// Stack onto an existing entry of the same definition.
	if (Count > 0 && Definition->IsStackable())
	{
		if (UFortWorldItem* Existing = Inventory->NativeFindWorldItemByDefinition(Definition, true))
		{
			const int32 MaxStack = Definition->NativeGetMaxStackSize(nullptr);
			const int32 Room = FMath::Max(0, MaxStack - Existing->ItemEntry.Count);
			const int32 Added = FMath::Min(Room, Count);
			if (Added > 0)
			{
				Existing->ItemEntry.Count += Added;
				if (FFortItemEntry* Replicated = FindReplicatedEntry(Inventory, Existing->ItemEntry.ItemGuid))
				{
					Replicated->Count = Existing->ItemEntry.Count;
					Inventory->Inventory.MarkItemDirty(*Replicated);
				}
				Inventory->NativeNotifyInventoryChanged();
				UE_LOG(LogTemp, Display, TEXT("Inventory: stacked %d x %s onto %s (now %d)"), Added, *Definition->GetName(), *Existing->ItemEntry.ItemGuid.ToString(), Existing->ItemEntry.Count);
			}
			Count -= Added;
			if (Count == 0 || !Definition->bAllowMultipleStacks)
			{
				if (Count > 0)
				{
					UE_LOG(LogTemp, Display, TEXT("Inventory: %d x %s did not fit (stack full, single stack only)"), Count, *Definition->GetName());
				}
				return Existing;
			}
		}
	}

	// Value-initialise: the generated structs declare their pointer members without initialisers, so a plain
	// `FFortItemEntry Entry;` leaves FFortGiftingInfo::HeroType as stack garbage and GC then fails with "Invalid object".
	FFortItemEntry Entry{};
	Entry.ItemDefinition = Definition;
	Entry.Count = Count;
	Entry.Level = Level;
	Entry.LoadedAmmo = LoadedAmmo;
	Entry.ItemGuid = FGuid::NewGuid();

	SnapInv(Inventory, TEXT("before NewObject"));
	UFortWorldItem* Item = NewObject<UFortWorldItem>(Inventory);
	SnapInv(Inventory, TEXT("after NewObject"));
	Item->OwnerInventory = Inventory;
	Item->ItemEntry = Entry;
	SnapInv(Inventory, TEXT("after ItemEntry copy"));
	UFortWorldItem::NativeSetEntryParentInventory(Item->ItemEntry, Inventory, false);
	SnapInv(Inventory, TEXT("after SetParent(item)"));
	// stand-in: the original creates the item through UFortItemDefinition::CreateTemporaryItemInstanceBP, which picks the
	// item class from the definition. A plain UFortWorldItem is used here (not verified against the original item classes).

	const int32 Index = Inventory->Inventory.ReplicatedEntries.Add(Entry);
	SnapInv(Inventory, TEXT("after ReplicatedEntries.Add"));
	FFortItemEntry& Replicated = Inventory->Inventory.ReplicatedEntries[Index];
	UFortWorldItem::NativeSetEntryParentInventory(Replicated, Inventory, true);
	SnapInv(Inventory, TEXT("after SetParent(replicated)"));
	Inventory->Inventory.ItemInstances.Add(Item);
	SnapInv(Inventory, TEXT("after ItemInstances.Add"));
	RebuildGuidIndex(Inventory);
	SnapInv(Inventory, TEXT("after RebuildGuidIndex"));
	Inventory->Inventory.MarkItemDirty(Inventory->Inventory.ReplicatedEntries[Index]);
	SnapInv(Inventory, TEXT("after MarkItemDirty"));
	Inventory->NativeNotifyInventoryChanged();
	SnapInv(Inventory, TEXT("after NativeNotify"));
	ScanInventory(Inventory, Item, TEXT("after"));

	UE_LOG(LogTemp, Display, TEXT("Inventory: granted %d x %s guid=%s level=%d to %s"),
		Count, *Definition->GetName(), *Entry.ItemGuid.ToString(), Level, *Controller->GetName());
	return Item;
}

int32 FortAthenaInventory::RemoveItem(AFortPlayerController* Controller, const FGuid& ItemGuid, int32 Count)
{
	AFortInventory* Inventory = Controller ? Controller->WorldInventory : nullptr;
	if (!IsEnabled() || !Inventory)
	{
		return 0;
	}
	UFortWorldItem* Item = FindItemByGuid(Inventory, ItemGuid);
	if (!Item)
	{
		UE_LOG(LogTemp, Display, TEXT("Inventory: remove %s ignored: unknown guid"), *ItemGuid.ToString());
		return 0;
	}
	const int32 Removed = (Count <= 0) ? Item->ItemEntry.Count : FMath::Min(Count, Item->ItemEntry.Count);
	Item->ItemEntry.Count -= Removed;
	if (Item->ItemEntry.Count > 0)
	{
		if (FFortItemEntry* Replicated = FindReplicatedEntry(Inventory, ItemGuid))
		{
			Replicated->Count = Item->ItemEntry.Count;
			Inventory->Inventory.MarkItemDirty(*Replicated);
		}
	}
	else
	{
		if (AFortPawn* Pawn = GetControllerPawn(Controller))
		{
			const TArray<AFortWeapon*> Weapons = Pawn->CurrentWeaponList;
			for (AFortWeapon* Weapon : Weapons)
				if (Weapon && Weapon->ItemEntryGuid == ItemGuid)
					FortAthenaCombatEquip::RemoveWeapon(Weapon);
		}
		Inventory->Inventory.NativeEntriesByGuid.Remove(ItemGuid);
		for (int32 Index = 0; Index < Inventory->Inventory.ReplicatedEntries.Num(); ++Index)
		{
			if (Inventory->Inventory.ReplicatedEntries[Index].ItemGuid == ItemGuid)
			{
				Inventory->Inventory.ReplicatedEntries.RemoveAt(Index);
				break;
			}
		}
		Inventory->Inventory.ItemInstances.Remove(Item);
		Item->NativeDetachFromInventory();
		RebuildGuidIndex(Inventory);
		Inventory->Inventory.MarkArrayDirty();
	}
	Inventory->NativeNotifyInventoryChanged();
	UE_LOG(LogTemp, Display, TEXT("Inventory: removed %d of %s"), Removed, *ItemGuid.ToString());
	return Removed;
}

void FortAthenaInventory::ExecuteInventoryItem(AFortPlayerController* Controller, const FGuid& ItemGuid)
{
	if (!IsEnabled())
	{
		return;
	}
	AFortInventory* Inventory = Controller ? Controller->WorldInventory : nullptr;
	AFortPawn* Pawn = GetControllerPawn(Controller);
	if (!Inventory || !Pawn)
	{
		UE_LOG(LogTemp, Display, TEXT("Inventory: execute %s ignored: inventory=%d pawn=%d"), *ItemGuid.ToString(), Inventory ? 1 : 0, Pawn ? 1 : 0);
		return;
	}
	UFortWorldItem* Item = FindItemByGuid(Inventory, ItemGuid);
	if (!Item)
	{
		UE_LOG(LogTemp, Display, TEXT("Inventory: execute %s ignored: unknown guid"), *ItemGuid.ToString());
		return;
	}
	UE_LOG(LogTemp, Display, TEXT("Inventory: ServerExecuteInventoryItem %s (%s)"), *ItemGuid.ToString(),
		Item->ItemEntry.ItemDefinition ? *Item->ItemEntry.ItemDefinition->GetName() : TEXT("null"));
	EquipItem(Controller, Pawn, Item);
}

void FortAthenaInventory::Tick(UWorld* World)
{
	if (!IsEnabled() || !World)
		return;
	FortAthenaCombatEquip::Prune();
	// Resolution failures retry at most once per second per current weapon.
	static TMap<TWeakObjectPtr<AFortWeapon>, double> NextAudit;
	for (auto It = NextAudit.CreateIterator(); It; ++It) if (!It.Key().IsValid()) It.RemoveCurrent();
	auto& Sent = ClientEquipped();
	for (auto It = Sent.CreateIterator(); It; ++It)
		if (!It.Key().IsValid())
			It.RemoveCurrent();
	for (TActorIterator<AFortPawn> It(World); It; ++It)
	{
		AFortPawn* Pawn = *It;
		AFortWeapon* Weapon = Pawn->CurrentWeapon;
		if (Weapon && Pawn->HasAuthority() && World->GetTimeSeconds() >= NextAudit.FindOrAdd(Weapon))
		{
			NextAudit.FindOrAdd(Weapon) = World->GetTimeSeconds() + 1.0;
			AFortPlayerController* Controller = Cast<AFortPlayerController>(Pawn->GetController());
			UFortWorldItem* Item = Controller && Controller->WorldInventory ? FindItemByGuid(Controller->WorldInventory, Weapon->ItemEntryGuid) : nullptr;
			UFortWeaponItemDefinition* Definition = Item ? Cast<UFortWeaponItemDefinition>(Item->ItemEntry.ItemDefinition) : nullptr;
			if (Definition) GrantWeaponAbilities(Controller, Pawn, Weapon, Definition, true);
		}
		const ACharacter* Character = Cast<ACharacter>(Pawn);
		const UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
		if (!Weapon || !Pawn->HasAuthority() || !Pawn->IsPlayerControlled() || !Movement
			|| Movement->MovementMode != MOVE_Walking || !Movement->IsMovingOnGround())
			continue;
		TWeakObjectPtr<AFortWeapon>& Last = Sent.FindOrAdd(Pawn);
		if (Last.Get() == Weapon)
			continue;
		Last = Weapon;
		Pawn->ClientInternalEquipWeapon(Weapon);
		UE_LOG(LogTemp, Display, TEXT("Inventory: ClientInternalEquipWeapon %s on %s"), *Weapon->GetName(), *Pawn->GetName());
	}
}

void FortAthenaInventory::SetupPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Controller)
{
	if (!IsEnabled() || !Controller)
	{
		return;
	}
	AFortInventory* Inventory = Controller->WorldInventory;
	if (!Inventory)
	{
		// EnsureWorldInventory (FortniteAthenaGameplay.cpp) runs before the pawn spawns; it is never replaced here.
		UE_LOG(LogTemp, Warning, TEXT("Inventory: %s has no WorldInventory; nothing granted"), *Controller->GetName());
		return;
	}

	const bool bFirstSetup = Inventory->Inventory.ItemInstances.Num() == 0;
	if (bFirstSetup)
	{
		FString Source;
		if (UFortItemDefinition* Pickaxe = ResolvePickaxe(Controller, Source))
		{
			UE_LOG(LogTemp, Display, TEXT("Inventory: pickaxe %s from %s"), *Pickaxe->GetName(), *Source);
			if (UFortWorldItem* Item = GiveItem(Controller, Pickaxe, 1))
			{
				PickaxeGuids().Add(Controller, Item->ItemEntry.ItemGuid);
			}
		}

		int32 Starting = 0;
		if (GameMode && GameMode->StartingItems.Num() > 0)
		{
			for (const FItemAndCount& Start : GameMode->StartingItems)
			{
				int32 StartCount = Start.Count;
				int32 DebugWood = 0;
				// stand-in: -FortniteStartingWood=N (see below) replaces the empty wood stack for building tests.
				if (Start.Item && StartCount == 0 && Start.Item->GetName() == TEXT("WoodItemData") && FParse::Value(FCommandLine::Get(), TEXT("FortniteStartingWood="), DebugWood) && DebugWood > 0)
					StartCount = DebugWood;
				if (Start.Item && GiveItem(Controller, Start.Item, StartCount))
				{
					++Starting;
				}
			}
			UE_LOG(LogTemp, Display, TEXT("Inventory: %d starting items from the game mode (%s)"), Starting, *GameMode->GetClass()->GetName());
		}
		else
		{
			// stand-in: the native FortGameModeAthena has no Blueprint StartingItems; use the Athena_GameMode list (C card).
			UE_LOG(LogTemp, Display, TEXT("Inventory: game mode %s has no StartingItems; using the 12.60 Athena_GameMode list"),
				GameMode ? *GameMode->GetClass()->GetName() : TEXT("null"));
			for (const FDefaultStartItem& Start : GDefaultStartingItems)
			{
				if (UFortItemDefinition* Definition = LoadDefinition(Start.Path))
				{
					// stand-in: -FortniteStartingWood=N gives that much wood at spawn so building can be tested before harvesting exists.
					int32 StartCount = Start.Count;
					int32 DebugWood = 0;
					if (StartCount == 0 && FParse::Value(FCommandLine::Get(), TEXT("FortniteStartingWood="), DebugWood) && DebugWood > 0
						&& FString(Start.Path).Contains(TEXT("WoodItemData")))
					{
						StartCount = DebugWood;
					}
					if (GiveItem(Controller, Definition, StartCount))
					{
						++Starting;
					}
				}
			}
		}
		UE_LOG(LogTemp, Display, TEXT("Inventory: setup for %s: %d items in the world inventory (%d starting items)"),
			*Controller->GetName(), Inventory->Inventory.ItemInstances.Num(), Starting);
	}

	// Equip the pickaxe (the original client then shows it as the held item; slot 0 of the primary quickbar).
	AFortPawn* Pawn = GetControllerPawn(Controller);
	const FGuid* PickaxeGuid = PickaxeGuids().Find(Controller);
	if (!Pawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory: %s has no pawn yet; pickaxe not equipped"), *Controller->GetName());
	}
	else if (!PickaxeGuid)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory: %s has no pickaxe item; nothing to equip"), *Controller->GetName());
	}
	else if (!Pawn->CurrentWeapon)
	{
		ExecuteInventoryItem(Controller, *PickaxeGuid);
	}
}

// Global-namespace entry points for behaviour snippets (.inc bodies cannot use a qualified block-scope declaration).
UFortWorldItem* FortAthenaInventory_GiveItem(AFortPlayerController* Controller, UFortItemDefinition* Definition, int32 Count, int32 Level, int32 LoadedAmmo)
{
	return FortAthenaInventory::GiveItem(Controller, Definition, Count, Level, LoadedAmmo);
}

void FortAthenaInventory_ExecuteInventoryItem(AFortPlayerController* Controller, const FGuid& ItemGuid)
{
	FortAthenaInventory::ExecuteInventoryItem(Controller, ItemGuid);
}

namespace
{
	// Exec thunk for AFortPlayerController::ServerExecuteInventoryItem(FGuid). Same parameter layout as the generated
	// execServerExecuteInventoryItem; the _Validate step is the shared "return true" stub (0x1406801d0), so skipping it
	// changes nothing.
	void ExecServerExecuteInventoryItemOverride(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_STRUCT(FGuid, ItemGuid);
		P_FINISH;
		P_NATIVE_BEGIN;
		FortAthenaInventory::ExecuteInventoryItem(Cast<AFortPlayerController>(Context), ItemGuid);
		P_NATIVE_END;
	}
}

void FortAthenaInventory::InstallRpcOverrides()
{
	if (!IsEnabled())
		return;
	static bool bInstalled = false;
	if (bInstalled)
		return;
	if (UFunction* Function = AFortPlayerController::StaticClass()->FindFunctionByName(TEXT("ServerExecuteInventoryItem")))
	{
		Function->SetNativeFunc(&ExecServerExecuteInventoryItemOverride);
		bInstalled = true;
		UE_LOG(LogTemp, Display, TEXT("Inventory: ServerExecuteInventoryItem rebound to the re-created body"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Inventory: ServerExecuteInventoryItem UFunction not found; equip RPC stays empty"));
	}
}
