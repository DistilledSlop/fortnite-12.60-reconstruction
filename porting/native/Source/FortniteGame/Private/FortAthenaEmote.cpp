// Emote playback re-creation.
//   binary AFortPlayerController::ServerPlayEmoteItem_Implementation 0x1406801c0 = shared bare RET (stripped server body).
//   data   /Game/Abilities/Emotes/GAB_Emote_Generic is the generic emote ability; the emote item is the ability's
//          SourceObject (the ability reads the montage from it). UAthenaDanceItemDefinition::CustomDanceAbility overrides
//          the generic ability for special emotes (battle horn, thigh slapper, ...).
//   stand-in: the original validation (owned/unlocked emote, pawn state) is not recovered. The emote is allowed whenever
//          the pawn is alive; the ability's own activation tags decide whether it can start (not while skydiving etc.).
//   stand-in: sprays/toys use the same path with their own generic ability when the item class says so is not ported;
//          only dance items are handled, anything else is logged.
#include "FortAthenaEmote.h"
#include "FortAthenaHealth.h"
#include "AFortPlayerController.h"
#include "AFortPawn.h"
#include "UAthenaDanceItemDefinition.h"
#include "UFortMontageItemDefinitionBase.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "UObject/ScriptMacros.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortEmote, Log, All);

namespace
{
	void ExecServerPlayEmoteItem(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(UFortMontageItemDefinitionBase, EmoteAsset);
		P_GET_PROPERTY(FFloatProperty, EmoteRandomNumber);
		P_FINISH;
		P_NATIVE_BEGIN;
		AFortPlayerController* PC = Cast<AFortPlayerController>(Context);
		AFortPawn* Pawn = PC ? Cast<AFortPawn>(PC->GetPawn()) : nullptr;
		UAbilitySystemComponent* ASC = Pawn ? Pawn->GetAbilitySystemComponent() : nullptr;
		if (!EmoteAsset || !Pawn || !ASC || Pawn->bIsDying)
		{
			UE_LOG(LogFortEmote, Display, TEXT("Emote: ignored (asset %s pawn %s asc %s)"), *GetNameSafe(EmoteAsset), *GetNameSafe(Pawn), ASC ? TEXT("yes") : TEXT("no"));
		}
		else
		{
			UClass* AbilityClass = nullptr;
			if (const UAthenaDanceItemDefinition* Dance = Cast<UAthenaDanceItemDefinition>(EmoteAsset))
			{
				if (!Dance->CustomDanceAbility.IsNull())
					AbilityClass = Dance->CustomDanceAbility.LoadSynchronous();
				if (!AbilityClass)
					AbilityClass = LoadClass<UGameplayAbility>(nullptr, TEXT("/Game/Abilities/Emotes/GAB_Emote_Generic.GAB_Emote_Generic_C"));
			}
			if (!AbilityClass)
			{
				UE_LOG(LogFortEmote, Warning, TEXT("Emote: %s (%s) has no ability path (not a dance item)"), *GetNameSafe(EmoteAsset), *GetNameSafe(EmoteAsset->GetClass()));
			}
			else
			{
				FGameplayAbilitySpec Spec(AbilityClass->GetDefaultObject<UGameplayAbility>(), 1, INDEX_NONE, EmoteAsset);
				const FGameplayAbilitySpecHandle Handle = ASC->GiveAbilityAndActivateOnce(Spec);
				UE_LOG(LogFortEmote, Display, TEXT("Emote: %s plays %s through %s (random %.3f, handle %s)"), *GetNameSafe(Pawn), *GetNameSafe(EmoteAsset),
					*GetNameSafe(AbilityClass), EmoteRandomNumber, Handle.IsValid() ? TEXT("valid") : TEXT("INVALID"));
			}
		}
		P_NATIVE_END;
	}
}

void FortAthenaEmote::InstallRpcOverrides()
{
	static bool bInstalled = false;
	if (bInstalled || !FortAthenaHealth::IsEnabled())
		return;
	bInstalled = true;
	if (UFunction* Function = AFortPlayerController::StaticClass()->FindFunctionByName(TEXT("ServerPlayEmoteItem")))
	{
		Function->SetNativeFunc(&ExecServerPlayEmoteItem);
		UE_LOG(LogFortEmote, Display, TEXT("Emote: AFortPlayerController::ServerPlayEmoteItem rebound to the re-created body"));
	}
	else
	{
		UE_LOG(LogFortEmote, Warning, TEXT("Emote: ServerPlayEmoteItem UFunction not found"));
	}
}
