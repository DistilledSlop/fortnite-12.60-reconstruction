#pragma once
#include "GameplayTagsManager.h"

// Native tag-adder145140080 registers seven tags beneath UI.State.Athena.
// State virtual490 also consumes the separate Startup child/prefix adders.
struct FStartupRootNativeTags : public FGameplayTagNativeAdder
{
	FGameplayTag AthenaFrontend;
	FGameplayTag AthenaBattleRoyale;
	FGameplayTag AthenaSpectator;
	FGameplayTag AthenaReplay;
	FGameplayTag AthenaDevCustomGame;
	FGameplayTag AthenaCreative;
	FGameplayTag AthenaRejoin;
	FGameplayTag Athena;
	FGameplayTag Startup;
	FGameplayTag StartupLogin;
	FGameplayTag StartupSubgameSelect;
	FGameplayTag StartupRejoin;

	static FStartupRootNativeTags& Get();
	virtual void AddTags() override
	{
		auto& Manager = UGameplayTagsManager::Get();
		AthenaFrontend = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Athena.Frontend")), TEXT("(Native)"));
		AthenaBattleRoyale = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Athena.BattleRoyale")), TEXT("(Native)"));
		AthenaSpectator = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Athena.Spectator")), TEXT("(Native)"));
		AthenaReplay = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Athena.Replay")), TEXT("(Native)"));
		AthenaDevCustomGame = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Athena.Dev_CustomGame")), TEXT("(Native)"));
		AthenaCreative = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Athena.Creative")), TEXT("(Native)"));
		AthenaRejoin = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Athena.Rejoin")), TEXT("(Native)"));
		// Original prefix/Startup registrations are separate native adders;
		// integration must share these with the manager's native tag provider.
		Athena = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Athena")), TEXT("(Native)"));
		Startup = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Startup")), TEXT("(Native)"));
		StartupLogin = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Startup.Login")), TEXT("(Native)"));
		StartupSubgameSelect = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Startup.SubgameSelect")), TEXT("(Native)"));
		StartupRejoin = Manager.AddNativeGameplayTag(FName(TEXT("UI.State.Startup.Rejoin")), TEXT("(Native)"));
	}
};
