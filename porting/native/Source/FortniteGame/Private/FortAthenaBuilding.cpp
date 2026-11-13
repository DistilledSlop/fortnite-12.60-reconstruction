#include "FortAthenaBuilding.h"

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

	// stand-in: the client's class handle indexes AllPlayerBuildableClasses, which the client builds from its asset registry
	// (binary 0x141e92db0 / 0x141df9620) and the server cannot reproduce with a partial staged set. The held building tool
	// (replicated CurrentWeapon, set by ServerExecuteInventoryItem) names the piece instead: wood tier 1.
	UClass* ResolveHeldPiece(AFortPlayerController* Controller, FString& OutTool)
	{
		AFortPawn* Pawn = Controller ? Controller->GetPawn<AFortPawn>() : nullptr;
		AFortWeapon* Weapon = Pawn ? Pawn->CurrentWeapon : nullptr;
		const UFortItemDefinition* Definition = Weapon ? Weapon->WeaponData : nullptr;
		if (!Definition)
			return nullptr;
		OutTool = Definition->GetName();
		const TCHAR* Path = nullptr;
		if (OutTool == TEXT("BuildingItemData_Wall")) Path = TEXT("/Game/Building/ActorBlueprints/Player/Wood/L1/PBWA_W1_Solid.PBWA_W1_Solid_C");
		else if (OutTool == TEXT("BuildingItemData_Floor")) Path = TEXT("/Game/Building/ActorBlueprints/Player/Wood/L1/PBWA_W1_Floor.PBWA_W1_Floor_C");
		else if (OutTool == TEXT("BuildingItemData_Stair_W")) Path = TEXT("/Game/Building/ActorBlueprints/Player/Wood/L1/PBWA_W1_StairW.PBWA_W1_StairW_C");
		else if (OutTool == TEXT("BuildingItemData_RoofS")) Path = TEXT("/Game/Building/ActorBlueprints/Player/Wood/L1/PBWA_W1_RoofS.PBWA_W1_RoofS_C");
		return Path ? StaticLoadClass(ABuildingSMActor::StaticClass(), nullptr, Path) : nullptr;
	}

	void CreateBuilding(AFortPlayerController* Controller, const FCreateBuildingActorData& Data)
	{
		if (!Enabled() || !Controller || !Controller->HasAuthority())
			return;
		UWorld* World = Controller->GetWorld();
		FString Tool;
		UClass* Class = ResolveHeldPiece(Controller, Tool);
		if (!Class)
		{
			UE_LOG(LogTemp, Warning, TEXT("Building: refused handle %u: held item '%s' is not a known building tool"), Data.BuildingClassHandle, *Tool);
			return;
		}
		const ABuildingSMActor* Default = Cast<ABuildingSMActor>(Class->GetDefaultObject());
		const bool bInfinite = FParse::Param(FCommandLine::Get(), TEXT("FortniteInfiniteBuildMaterials"));
		UFortWorldItem* Wood = nullptr;
		if (Controller->WorldInventory)
		{
			for (UFortWorldItem* Item : Controller->WorldInventory->Inventory.ItemInstances)
			{
				const UFortResourceItemDefinition* Resource = Item ? Cast<UFortResourceItemDefinition>(Item->ItemEntry.ItemDefinition) : nullptr;
				if (Resource && Resource->ResourceType == EFortResourceType::Wood && Item->ItemEntry.Count > 0) { Wood = Item; break; }
			}
		}
		if (!bInfinite && (!Wood || Wood->ItemEntry.Count < BuildCost))
		{
			UE_LOG(LogTemp, Display, TEXT("Building: refused %s: %d wood available, %d required"), *Class->GetName(), Wood ? Wood->ItemEntry.Count : 0, BuildCost);
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
		Piece->bPlayerPlaced = 1;
		Piece->bMirrored = Data.bMirrored ? 1 : 0;
		if (const AFortPlayerStateAthena* PS = Cast<AFortPlayerStateAthena>(Controller->PlayerState))
			Piece->SetTeam(PS->TeamIndex);
		Piece->ForceNetUpdate();
		if (!bInfinite && Wood)
			FortAthenaInventory::RemoveItem(Controller, Wood->ItemEntry.ItemGuid, BuildCost);
		UE_LOG(LogTemp, Display, TEXT("Building: placed %s (%s) at %s yaw %.0f mirrored %d wood left %d"), *Piece->GetName(), *Tool,
			*FVector(Data.BuildLoc).ToString(), Data.BuildRot.Yaw, Data.bMirrored ? 1 : 0, Wood ? Wood->ItemEntry.Count : -1);
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
	// binary 0x141e92db0 / 0x141df9620: enumerate the object library's
	// bIsPlayerBuildable entries, then sort them. A partial staged set cannot
	// reproduce the client's handles; do not create a misleading table from it.
	UE_LOG(LogTemp, Warning, TEXT("Building: class table has %d entries; complete client-equivalent enumeration and ordering are unresolved"),
		GameState->AllPlayerBuildableClasses.Num());
}
