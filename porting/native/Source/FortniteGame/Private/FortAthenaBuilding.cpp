#include "FortAthenaBuilding.h"
#include "FortBuildingClassCache.h"
#include "FortAthenaBuildDamage.h"

#include "AFortGameStateAthena.h"
#include "AFortInventory.h"
#include "AFortPlayerController.h"
#include "ABuildingSMActor.h"
#include "FCreateBuildingActorData.h"
#include "UFortResourceItemDefinition.h"
#include "UFortWorldItem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
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
	// Largest builder-to-piece distance accepted (preview reach is a few grid cells; generous to absorb client/server position lag).
	constexpr float MaxBuildReach = 2500.0f;
	// Two pieces of one type whose centres are closer than this occupy the same grid slot (grid cells are 512 wide).
	constexpr float SlotMatchTolerance = 16.0f;

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
		// Placement validation. The client snaps its preview to the building grid and only sends placements it considers valid; the server
		// repeats the two checks that do not need the (unrecovered) structural grid: the piece must be within build reach of the
		// builder, and its grid slot must be free (no live piece of the same type centred on the same spot).
		const FVector BuildLocation(Data.BuildLoc);
		const AFortPawn* Builder = Controller->GetPawn<AFortPawn>();
		if (Builder && FVector::DistSquared(Builder->GetActorLocation(), BuildLocation) > FMath::Square(MaxBuildReach))
		{
			UE_LOG(LogTemp, Display, TEXT("Building: refused %s: %.0f units from the builder (reach %.0f)"), *Class->GetName(),
				FVector::Dist(Builder->GetActorLocation(), BuildLocation), MaxBuildReach);
			return;
		}
		for (TActorIterator<ABuildingSMActor> It(World); It; ++It)
		{
			const ABuildingSMActor* Existing = *It;
			if (IsValid(Existing) && !Existing->bDestroyed && Existing->BuildingType == Default->BuildingType
				&& FVector::DistSquared(Existing->GetActorLocation(), BuildLocation) < FMath::Square(SlotMatchTolerance))
			{
				UE_LOG(LogTemp, Display, TEXT("Building: refused %s: grid slot already holds %s"), *Class->GetName(), *Existing->GetName());
				return;
			}
		}
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		// stand-in: no structural support propagation (binary 0x1423881c0 not ported); the client already snaps its preview, so its
		// BuildLoc is used as sent once the checks above pass.
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
		FortAthenaBuildDamage::InitializePlayerBuiltPiece(Piece, Controller);
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
