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
	// validation are not recovered. Refuse multi-item and partially fitting pickups.
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
			const FFortItemEntry& Entry = Pickup->PrimaryPickupItemEntry;
			UFortItemDefinition* Definition = Entry.ItemDefinition;
			bool bFits = Definition && Entry.Count > 0;
			if (bFits && Definition->IsStackable())
			{
				const int32 MaxStack = Definition->NativeGetMaxStackSize(nullptr);
				UFortWorldItem* Existing = Inventory->NativeFindWorldItemByDefinition(Definition, true);
				// GiveItem returns success even when a single stack accepted only part of the count.
				bFits = MaxStack > 0 && Entry.Count <= MaxStack;
				if (Existing)
					bFits = bFits && Entry.Count <= FMath::Max(0, MaxStack - Existing->ItemEntry.Count);
			}
			if (bFits)
			{
				Pickup->bPickedUp = true;
				if (FortAthenaInventory::GiveItem(Controller, Definition, Entry.Count, Entry.Level, Entry.LoadedAmmo))
					Pickup->Destroy();
				else
					Pickup->bPickedUp = false;
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
