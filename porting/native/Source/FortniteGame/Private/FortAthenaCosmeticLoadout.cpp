// stand-in: the original server copies the player's whole cosmetic loadout (FFortAthenaLoadout) from the profile into
// AFortPlayerController::CosmeticLoadoutPC and the pawn. Only the character was ported (FortAthenaCharacterCache); glider,
// pickaxe, backpack, contrail and emotes stayed at their defaults, which is why a chosen glider never appeared.
// Not ported: item variants (styles) of these slots: FFortAthenaLoadout has no field for them, the original carries them
// through another path (AFortPlayerState::ServerChangeActiveVariantForCosmeticItem, stripped).
#include "FortAthenaCosmeticLoadout.h"
#include "AFortPlayerController.h"
#include "AFortPlayerPawn.h"
#include "FFortAthenaLoadout.h"
#include "UAthenaGliderItemDefinition.h"
#include "UAthenaPickaxeItemDefinition.h"
#include "UAthenaBackpackItemDefinition.h"
#include "UAthenaSkyDiveContrailItemDefinition.h"
#include "UAthenaDanceItemDefinition.h"
#include "UAthenaItemWrapDefinition.h"
#include "UAthenaMusicPackItemDefinition.h"
#include "UAthenaVictoryPoseItemDefinition.h"
#include "UAthenaLoadingScreenItemDefinition.h"
#include "AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortCosmeticLoadout, Log, All);

namespace
{
	struct FCosmeticFolder
	{
		const TCHAR* Category;
		const TCHAR* Folder;
	};
	const FCosmeticFolder GCosmeticFolders[] = {
		{TEXT("Glider"), TEXT("/Game/Athena/Items/Cosmetics/Gliders")},
		{TEXT("Pickaxe"), TEXT("/Game/Athena/Items/Cosmetics/Pickaxes")},
		{TEXT("Backpack"), TEXT("/Game/Athena/Items/Cosmetics/Backpacks")},
		{TEXT("SkyDiveContrail"), TEXT("/Game/Athena/Items/Cosmetics/Contrails")},
		{TEXT("Dance"), TEXT("/Game/Athena/Items/Cosmetics/Dances")},
		{TEXT("ItemWrap"), TEXT("/Game/Athena/Items/Cosmetics/ItemWraps")},
		{TEXT("MusicPack"), TEXT("/Game/Athena/Items/Cosmetics/MusicPacks")},
		{TEXT("VictoryPose"), TEXT("/Game/Athena/Items/Cosmetics/VictoryPoses")},
		{TEXT("LoadingScreen"), TEXT("/Game/Athena/Items/Cosmetics/LoadingScreens")},
	};

	bool SafeName(const FString& Name)
	{
		if (Name.IsEmpty() || Name.Len() > 128)
			return false;
		for (const TCHAR Ch : Name)
			if (!(FChar::IsAlnum(Ch) || Ch == TEXT('_') || Ch == TEXT('-')))
				return false;
		return true;
	}

	// lower-case asset name -> object path, per category, from the asset registry (one scan per folder).
	const TMap<FString, FString>& NameMap(const TCHAR* Folder)
	{
		static TMap<FString, TMap<FString, FString>> Cache;
		const FString Key(Folder);
		if (TMap<FString, FString>* Found = Cache.Find(Key))
			return *Found;
		TMap<FString, FString>& Map = Cache.Add(Key);
		IAssetRegistry& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
		TArray<FAssetData> Assets;
		Registry.GetAssetsByPath(FName(Folder), Assets, true);
		for (const FAssetData& Asset : Assets)
			Map.Add(Asset.AssetName.ToString().ToLower(), Asset.ObjectPath.ToString());
		UE_LOG(LogFortCosmeticLoadout, Display, TEXT("Cosmetics: %s -> %d assets in the registry"), Folder, Map.Num());
		return Map;
	}

	template <typename T>
	T* LoadItem(const TCHAR* Category, const FString& TemplateId)
	{
		if (TemplateId.IsEmpty())
			return nullptr;
		FString Name = TemplateId;
		int32 Colon;
		if (Name.FindChar(TEXT(':'), Colon))
			Name = Name.Mid(Colon + 1);
		if (!SafeName(Name))
		{
			UE_LOG(LogFortCosmeticLoadout, Warning, TEXT("Cosmetics: %s '%s' rejected (unsafe characters)"), Category, *TemplateId);
			return nullptr;
		}
		for (const FCosmeticFolder& Entry : GCosmeticFolders)
		{
			if (FCString::Stricmp(Entry.Category, Category) != 0)
				continue;
			FString Path;
			if (const FString* Found = NameMap(Entry.Folder).Find(Name.ToLower()))
				Path = *Found;
			else
				Path = FString::Printf(TEXT("%s/%s.%s"), Entry.Folder, *Name, *Name);  // registry miss: plain path
			T* Item = LoadObject<T>(nullptr, *Path);
			if (!Item)
				UE_LOG(LogFortCosmeticLoadout, Warning, TEXT("Cosmetics: %s '%s' not loadable at %s"), Category, *TemplateId, *Path);
			return Item;
		}
		return nullptr;
	}

	void Fill(FFortAthenaLoadout& Out, const TMap<FString, TArray<FString>>& Slots)
	{
		auto First = [&Slots](const TCHAR* Category) -> FString
		{
			const TArray<FString>* Items = Slots.Find(Category);
			return (Items && Items->Num() > 0) ? (*Items)[0] : FString();
		};
		if (UAthenaGliderItemDefinition* Item = LoadItem<UAthenaGliderItemDefinition>(TEXT("Glider"), First(TEXT("Glider"))))
			Out.Glider = Item;
		if (UAthenaPickaxeItemDefinition* Item = LoadItem<UAthenaPickaxeItemDefinition>(TEXT("Pickaxe"), First(TEXT("Pickaxe"))))
			Out.Pickaxe = Item;
		if (UAthenaBackpackItemDefinition* Item = LoadItem<UAthenaBackpackItemDefinition>(TEXT("Backpack"), First(TEXT("Backpack"))))
			Out.Backpack = Item;
		if (UAthenaSkyDiveContrailItemDefinition* Item = LoadItem<UAthenaSkyDiveContrailItemDefinition>(TEXT("SkyDiveContrail"), First(TEXT("SkyDiveContrail"))))
			Out.SkyDiveContrail = Item;
		if (UAthenaItemWrapDefinition* Item = LoadItem<UAthenaItemWrapDefinition>(TEXT("ItemWrap"), First(TEXT("ItemWrap"))))
			Out.ItemWrapOverride = Item;
		if (UAthenaMusicPackItemDefinition* Item = LoadItem<UAthenaMusicPackItemDefinition>(TEXT("MusicPack"), First(TEXT("MusicPack"))))
			Out.MusicPack = Item;
		if (UAthenaVictoryPoseItemDefinition* Item = LoadItem<UAthenaVictoryPoseItemDefinition>(TEXT("VictoryPose"), First(TEXT("VictoryPose"))))
			Out.VictoryPose = Item;
		if (UAthenaLoadingScreenItemDefinition* Item = LoadItem<UAthenaLoadingScreenItemDefinition>(TEXT("LoadingScreen"), First(TEXT("LoadingScreen"))))
			Out.LoadingScreen = Item;
		if (const TArray<FString>* Dances = Slots.Find(TEXT("Dance")))
		{
			Out.Dances.Reset();
			for (const FString& Id : *Dances)
				Out.Dances.Add(LoadItem<UAthenaDanceItemDefinition>(TEXT("Dance"), Id));
		}
	}
}

void FortAthenaCosmeticLoadout::Apply(AController* Controller, const TMap<FString, TArray<FString>>& Slots)
{
	if (!Controller || Slots.Num() == 0)
		return;
	if (AFortPlayerController* FortPC = Cast<AFortPlayerController>(Controller))
		Fill(FortPC->CosmeticLoadoutPC, Slots);
	if (AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Controller->GetPawn()))
	{
		Fill(Pawn->CosmeticLoadout, Slots);
		Pawn->ForceNetUpdate();
		UE_LOG(LogFortCosmeticLoadout, Display, TEXT("Cosmetics: applied to %s glider=%s pickaxe=%s backpack=%s contrail=%s dances=%d"), *Pawn->GetName(),
			*GetNameSafe(Pawn->CosmeticLoadout.Glider), *GetNameSafe(Pawn->CosmeticLoadout.Pickaxe), *GetNameSafe(Pawn->CosmeticLoadout.Backpack),
			*GetNameSafe(Pawn->CosmeticLoadout.SkyDiveContrail), Pawn->CosmeticLoadout.Dances.Num());
	}
}
