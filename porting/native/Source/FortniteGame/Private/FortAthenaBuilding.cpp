#include "FortAthenaBuilding.h"
#include "FortBuildingClassCache.h"

#include "AFortGameStateAthena.h"
#include "AFortInventory.h"
#include "AFortPlayerController.h"
#include "ABuildingSMActor.h"
#include "FCreateBuildingActorData.h"
#include "UFortResourceItemDefinition.h"
#include "UFortWorldItem.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/ScriptMacros.h"
#include "AFortPlayerStateAthena.h"
#include "AFortPawn.h"
#include "AFortWeapon.h"
#include "UFortItemDefinition.h"
#include "UFortWeaponItemDefinition.h"
#include "FortAthenaInventoryFlow.h"

namespace
{
	bool Enabled()
	{
		return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
	}

	// stand-in: the stripped server body does not reveal the building cost.
	constexpr int32 BuildCost = 10;

	FString HeldToolName(AFortPlayerController* Controller)
	{
		AFortPawn* Pawn = Controller ? Controller->GetPawn<AFortPawn>() : nullptr;
		AFortWeapon* Weapon = Pawn ? Pawn->CurrentWeapon : nullptr;
		const UFortItemDefinition* Definition = Weapon ? Weapon->WeaponData : nullptr;
		return GetNameSafe(Definition);
	}

	void CreateBuilding(AFortPlayerController* Controller, const FCreateBuildingActorData& Data)
	{
		if (!Enabled() || !Controller || !Controller->HasAuthority())
			return;
		UWorld* World = Controller->GetWorld();
		if (!World) return;
		const FString Tool = HeldToolName(Controller);
		const AFortGameStateAthena* GameState = World ? World->GetGameState<AFortGameStateAthena>() : nullptr;
		UClass* Class = FortBuildingClassCache::Resolve(GameState, Data.BuildingClassHandle);
		UE_LOG(LogTemp, Display, TEXT("Building: request handle %u tool '%s' cached class '%s'"),
			Data.BuildingClassHandle, *Tool, *GetNameSafe(Class));
		if (!Class || !Class->IsChildOf(ABuildingSMActor::StaticClass()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Building: refused handle %u: class unavailable or not a building mesh"), Data.BuildingClassHandle);
			return;
		}
		const ABuildingSMActor* Default = Cast<ABuildingSMActor>(Class->GetDefaultObject());
		if (!Default) return;
		const bool bInfinite = FParse::Param(FCommandLine::Get(), TEXT("FortniteInfiniteBuildMaterials"));
		UFortWorldItem* Material = nullptr;
		if (Controller->WorldInventory)
		{
			for (UFortWorldItem* Item : Controller->WorldInventory->Inventory.ItemInstances)
			{
				const UFortResourceItemDefinition* Resource = Item ? Cast<UFortResourceItemDefinition>(Item->ItemEntry.ItemDefinition) : nullptr;
				if (Resource && Resource->ResourceType == Default->ResourceType && Item->ItemEntry.Count > 0) { Material = Item; break; }
			}
		}
		if (!bInfinite && (!Material || Material->ItemEntry.Count < BuildCost))
		{
			UE_LOG(LogTemp, Display, TEXT("Building: refused %s: %d resource type %d available, %d required"), *Class->GetName(), Material ? Material->ItemEntry.Count : 0, int32(Default->ResourceType.GetValue()), BuildCost);
			return;
		}
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// stand-in: no structural grid snap or overlap validation yet (binary 0x141c78270, 0x1423881c0 not ported); the
		// client already snaps its preview, so its BuildLoc is used as sent.
		ABuildingSMActor* Piece = World->SpawnActor<ABuildingSMActor>(Class, FVector(Data.BuildLoc), Data.BuildRot, Params);
		if (!Piece)
		{
			UE_LOG(LogTemp, Warning, TEXT("Building: spawn of %s failed at %s"), *Class->GetName(), *FVector(Data.BuildLoc).ToString());
			return;
		}
		// stand-in for the ABuildingActor constructor defaults (no generated constructor; see the weapon note in
		// FortAthenaInventoryFlow.cpp): placed pieces must replicate to every client.
		if (!Piece->GetIsReplicated())
			Piece->SetReplicates(true);
		Piece->bPlayerPlaced = 1;
		Piece->bMirrored = Data.bMirrored ? 1 : 0;
		if (const AFortPlayerStateAthena* PS = Cast<AFortPlayerStateAthena>(Controller->PlayerState))
			Piece->SetTeam(PS->TeamIndex);
		Piece->ForceNetUpdate();
		const int32 Remaining = Material ? Material->ItemEntry.Count - (bInfinite ? 0 : BuildCost) : -1;
		if (!bInfinite && Material)
			FortAthenaInventory::RemoveItem(Controller, Material->ItemEntry.ItemGuid, BuildCost);
		UE_LOG(LogTemp, Display, TEXT("Building: placed %s (%s) at %s yaw %.0f mirrored %d resource type %d left %d"), *Piece->GetName(), *Tool,
			*FVector(Data.BuildLoc).ToString(), Data.BuildRot.Yaw, Data.bMirrored ? 1 : 0, int32(Default->ResourceType.GetValue()), Remaining);
	}

	void ExecCreateBuilding(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_STRUCT(FCreateBuildingActorData, Data);
		P_FINISH;
		P_NATIVE_BEGIN;
		CreateBuilding(Cast<AFortPlayerController>(Context), Data);
		P_NATIVE_END;
	}
}

void FortAthenaBuilding::InstallRpcOverrides()
{
	if (!Enabled())
		return;
	static bool bInstalled = false;
	if (bInstalled)
		return;
	if (UFunction* Function = AFortPlayerController::StaticClass()->FindFunctionByName(TEXT("ServerCreateBuildingActor")))
	{
		Function->SetNativeFunc(&ExecCreateBuilding);
		bInstalled = true;
		UE_LOG(LogTemp, Display, TEXT("Building: ServerCreateBuildingActor rebound to the partial re-creation"));
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Building: ServerCreateBuildingActor UFunction not found"));
}

void FortAthenaBuilding::PrepareBuildableClasses(AFortGameStateAthena* GameState)
{
	if (!Enabled() || !GameState)
		return;
	// Original Athena client observed +0xe17 set, selecting level zero only.
	// The byte is unreflected and its playlist-dependent setter is not recovered;
	// this choice is an original-client Athena stand-in, not the constructor default.
	FortBuildingClassCache::Prepare(GameState, true);
}
