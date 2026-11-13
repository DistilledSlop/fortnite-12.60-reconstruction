#include "FortAthenaLoot.h"

#include "AFortPickupAthena.h"
#include "AFortInventory.h"
#include "AFortPlayerPawn.h"
#include "AFortPlayerController.h"
#include "FortAthenaInventoryFlow.h"
#include "UFortWorldItem.h"
#include "UObject/ScriptMacros.h"
#include "UFortItemDefinition.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

bool FortAthenaLoot::IsEnabled()
{
	return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
}

AFortPickupAthena* FortAthenaLoot::SpawnResolvedPickup(UWorld* World, const FVector& Location,
	UFortItemDefinition* Definition, int32 Count, int32 Level, int32 LoadedAmmo)
{
	if (!IsEnabled() || !World || World->GetNetMode() == NM_Client || !Definition || Count <= 0)
		return nullptr;

	// stand-in: native pickup selection, initialization and toss behavior still need recovery.
	// The result is not connected to a table roll or a map spawner yet.
	FActorSpawnParameters Parameters;
	Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFortPickupAthena* Pickup = World->SpawnActor<AFortPickupAthena>(Location, FRotator::ZeroRotator, Parameters);
	if (!Pickup)
	{
		UE_LOG(LogTemp, Warning, TEXT("Loot: failed to spawn resolved pickup"));
		return nullptr;
	}
	Pickup->PrimaryPickupItemEntry.ItemDefinition = Definition;
	Pickup->PrimaryPickupItemEntry.Count = Count;
	Pickup->PrimaryPickupItemEntry.Level = Level;
	Pickup->PrimaryPickupItemEntry.LoadedAmmo = LoadedAmmo;
	Pickup->PrimaryPickupItemEntry.ItemGuid = FGuid::NewGuid();
	Pickup->SetReplicates(true);
	Pickup->ForceNetUpdate();
	UE_LOG(LogTemp, Display, TEXT("Loot: spawned resolved pickup %s count=%d"), *Definition->GetName(), Count);
	return Pickup;
}

namespace
{
	// stand-in: immediate collection; original pickup flight, sound, slot/swap and interaction
	// validation are not recovered. Multi-item pickups remain unsupported; partial grants preserve the world remainder.
	void ExecServerHandlePickup(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(AFortPickup, Pickup);
		P_GET_PROPERTY(FFloatProperty, FlyTime);
		P_GET_STRUCT(FVector, StartDirection);
		P_GET_UBOOL(bPlayPickupSound);
		P_FINISH;
		P_NATIVE_BEGIN;
		AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Context);
		AFortPlayerController* Controller = Pawn ? Cast<AFortPlayerController>(Pawn->GetController()) : nullptr;
		AFortInventory* Inventory = Controller ? Controller->WorldInventory : nullptr;
		if (FortAthenaLoot::IsEnabled() && Pawn && Pawn->HasAuthority() && Inventory && IsValid(Pickup)
			&& Pickup->GetWorld() == Pawn->GetWorld() && !Pickup->bPickedUp
			&& Pickup->MultiItemPickupEntries.Num() == 0
			// stand-in: conservative range until original server pickup validation is recovered.
			&& FVector::DistSquared(Pawn->GetActorLocation(), Pickup->GetActorLocation()) <= FMath::Square(300.0f))
		{
			FFortItemEntry& Entry = Pickup->PrimaryPickupItemEntry;
			UFortItemDefinition* Definition = Entry.ItemDefinition;
			int32 GrantCount = Definition ? FMath::Max(0, Entry.Count) : 0;
			if (GrantCount > 0 && Definition->IsStackable())
			{
				const int32 MaxStack = Definition->NativeGetMaxStackSize(nullptr);
				// Bound a new entry to one stack. GiveItem can first fill an existing stack;
				// the remaining request still cannot create an oversized new entry.
				GrantCount = FMath::Min(GrantCount, FMath::Max(0, MaxStack));
				if (!Definition->bAllowMultipleStacks)
				{
					if (UFortWorldItem* Existing = Inventory->NativeFindWorldItemByDefinition(Definition, true))
						GrantCount = FMath::Min(GrantCount, FMath::Max(0, MaxStack - Existing->ItemEntry.Count));
				}
			}
			if (GrantCount > 0)
			{
				Pickup->bPickedUp = true;
				int32 AcceptedCount = 0;
				FortAthenaInventory::GiveItem(Controller, Definition, GrantCount, Entry.Level, Entry.LoadedAmmo, &AcceptedCount);
				// Consume exactly the amount committed to inventory; a returned item alone
				// does not establish that its existing stack accepted the entire request.
				check(AcceptedCount >= 0 && AcceptedCount <= GrantCount);
				Entry.Count -= AcceptedCount;
				if (Entry.Count == 0)
					Pickup->Destroy();
				else
				{
					Pickup->bPickedUp = false;
					if (AcceptedCount > 0)
						Pickup->ForceNetUpdate();
				}
			}

		}
		P_NATIVE_END;
	}
}

void FortAthenaLoot::InstallRpcOverrides()
{
	if (!IsEnabled())
		return;
	static bool bInstalled = false;
	if (bInstalled)
		return;
	if (UFunction* Function = AFortPlayerPawn::StaticClass()->FindFunctionByName(TEXT("ServerHandlePickup")))
	{
		Function->SetNativeFunc(&ExecServerHandlePickup);
		bInstalled = true;
		UE_LOG(LogTemp, Display, TEXT("Loot: ServerHandlePickup rebound (limited immediate collection)"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Loot: ServerHandlePickup UFunction missing"));
}
