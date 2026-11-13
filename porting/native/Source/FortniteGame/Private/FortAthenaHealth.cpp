// Server-side health, GAS (Ability System Component), attribute sets, damage, and death sequence
// for the original 12.60 client (WP G8-Health, SERVER_FIRST_PLAN.md).
//
// Evidence summary (labels: binary / data / source / inferred / stand-in):
// - binary 0x14195e620 (AFortGameModeAthena::RemoveFromAlivePlayers): decrements TeamAlivePlayers[team], sets
//   GameState->PlayersLeft, logs removal from alive players list, calls win check through 0x141937f90.
// - binary 0x141937590 (AFortGameModeAthena::CheckWinConditionLastManStanding): solo win condition requires
//   GamePhase in Aircraft or SafeZones, OldPlayersLeft >= 2, and PlayersLeft == 1; then calls StartEndGamePhase.
// - binary 0x14196cb30 (StartEndGamePhase): sets winner place = 1, writes GameState->WinningPlayerState, sends
//   ClientNotifyWon(FinisherPawn, FinishingWeapon, DeathCause) and match statistics.
// - binary 0x142334fc0 (AFortPawn::NetMulticast_Athena_BatchedDamageCues): hit markers, damage numbers, and shield
//   shatter cues multicast to clients.
// - binary 0x141a5f1c0 (AFortPlayerStateAthena::ToDeathCause): resolves damage tags to EDeathCause using
//   UFortDeathCauseFromTagMapping and native tag table offsets (0x360 = OutsideSafeZone, 0x358 = FallDamage).
// - binary 0x141a128d0 (AFortPawn::GetAbilitySystemComponent): returns AbilitySystemComponent member.
// - binary 0x14231c4c0 (AFortPawn::GetHealth): returns 1.0f unless bHealthSynced is true and HealthSet is non-null.
//   This explains the G6 symptom "health shows 0" on the client HUD when HealthSet is missing.
// - data /Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer (UFortAbilitySet): 31 player abilities
//   (jump, sprint, death, interact, vehicle, DBNO, etc.) and 2 default effects (GE_DefaultResourceCap_C,
//   GE_DefaultTurboBuildSettings_C).
// - source server_systems/D6-combat-and-elimination.md §2: server-side death sequence specification.
// - stand-in: marks behaviours that are stand-ins or re-creations of server logic compiled out of the client exe.

#include "FortAthenaHealth.h"
#include "UObject/ScriptMacros.h"

#include "AFortGameModeAthena.h"
#include "AFortGameStateAthena.h"
#include "AFortPawn.h"
#include "AFortPlayerControllerAthena.h"
#include "AFortPlayerPawn.h"
#include "AFortPlayerPawnAthena.h"
#include "AFortPlayerStateAthena.h"
#include "AFortWeapon.h"
#include "EAthenaGamePhase.h"
#include "EDeathCause.h"
#include "FAthenaBatchedDamageGameplayCues_NonShared.h"
#include "FAthenaBatchedDamageGameplayCues_Shared.h"
#include "FAthenaMatchStats.h"
#include "FAthenaMatchTeamStats.h"
#include "FAthenaRewardResult.h"
#include "FDeathInfo.h"
#include "FFortPlayerDeathReport.h"
#include "FortNativeGameplayTagsRuntime.h"
#include "UFortAbilitySet.h"
#include "UFortAbilitySystemComponent.h"
#include "UFortAdvancedMovementSet.h"
#include "UFortControlResistanceSet.h"
#include "UFortDamageSet.h"
#include "UFortGameplayAbility.h"
#include "UFortGameplayAbility_Jump.h"
#include "UFortGameplayAbility_Sprint.h"
#include "UFortHealthSet.h"
#include "UFortMovementSet.h"
#include "UFortRegenHealthSet.h"
#include "UFortWeaponItemDefinition.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameplayEffectExtension.h"
#include "GameplayEffectTypes.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortHealth, Log, All);

namespace
{
	// Tracks active, alive player controllers in the match.
	TArray<TWeakObjectPtr<AFortPlayerControllerAthena>>& GetAlivePlayers()
	{
		static TArray<TWeakObjectPtr<AFortPlayerControllerAthena>> List;
		return List;
	}

	// Tracks PlayerStates that have already been granted the default player ability set.
	TSet<TWeakObjectPtr<AFortPlayerState>>& GetGrantedPlayerStates()
	{
		static TSet<TWeakObjectPtr<AFortPlayerState>> Set;
		return Set;
	}

	// Tracks AbilitySystemComponents that have had the health change listener bound.
	TSet<TWeakObjectPtr<UAbilitySystemComponent>>& GetBoundASCs()
	{
		static TSet<TWeakObjectPtr<UAbilitySystemComponent>> Set;
		return Set;
	}

	// Rebind for AFortPlayerPawnAthena::ServerSuicide(bSuppressResurrectionChip, MatchPlacement).
	void ExecServerSuicideOverride(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_UBOOL(bSuppressResurrectionChip);
		P_GET_PROPERTY(FIntProperty, MatchPlacement);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(Context))
		{
			UE_LOG(LogFortHealth, Display, TEXT("Health: ServerSuicide received for %s"), *Pawn->GetName());
			FGameplayTagContainer SuicideTags;
			SuicideTags.AddTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x418));
			FortAthenaHealth::ApplyDamage(Pawn, Pawn->GetHealth() + Pawn->GetShield(), SuicideTags, Pawn->GetController(), Pawn, true);
		}
		P_NATIVE_END;
	}

	// Rebind for AFortPlayerPawn::ServerResetAbilitySystemComponent().
	void ExecServerResetAbilitySystemComponentOverride(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Context))
		{
			UE_LOG(LogFortHealth, Display, TEXT("Health: ServerResetAbilitySystemComponent received for %s"), *Pawn->GetName());
			if (AFortPlayerControllerAthena* PC = Cast<AFortPlayerControllerAthena>(Pawn->GetController()))
			{
				if (UWorld* World = Pawn->GetWorld())
				{
					AFortGameModeAthena* GM = Cast<AFortGameModeAthena>(World->GetAuthGameMode());
					FortAthenaHealth::SetupPlayer(GM, PC);
				}
			}
		}
		P_NATIVE_END;
	}
}

bool FortAthenaHealth::IsEnabled()
{
	return FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient"));
}

int32 FortAthenaHealth::GetAlivePlayerCount()
{
	TArray<TWeakObjectPtr<AFortPlayerControllerAthena>>& Alive = GetAlivePlayers();
	Alive.RemoveAll([](const TWeakObjectPtr<AFortPlayerControllerAthena>& P) { return !P.IsValid(); });
	return Alive.Num();
}

void FortAthenaHealth::SetupPlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Controller)
{
	if (!IsEnabled() || !GameMode || !Controller)
	{
		return;
	}

	AFortPlayerStateAthena* PlayerState = Cast<AFortPlayerStateAthena>(Controller->PlayerState);
	AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Controller->GetPawn());
	if (!PlayerState || !Pawn)
	{
		return;
	}

	// 1. Ensure PlayerState owns an active UFortAbilitySystemComponent.
	UFortAbilitySystemComponent* ASC = PlayerState->AbilitySystemComponent;
	if (!ASC)
	{
		ASC = NewObject<UFortAbilitySystemComponent>(PlayerState, TEXT("AbilitySystemComponent"));
		ASC->SetIsReplicated(true);
		ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
		PlayerState->AbilitySystemComponent = ASC;
		PlayerState->AddOwnedComponent(ASC);
		ASC->RegisterComponent();
		UE_LOG(LogFortHealth, Display, TEXT("Health: Created and registered AbilitySystemComponent on %s"), *PlayerState->GetName());
	}

	// 2. Initialize AbilityActorInfo: Owner is PlayerState, Avatar is Pawn.
	ASC->InitAbilityActorInfo(PlayerState, Pawn);
	Pawn->AbilitySystemComponent = ASC;
	Pawn->bInitAbilitySystemComponentFromPlayerState = 1;

	// 3. Ensure attribute sets exist on PlayerState/ASC and are mirrored on Pawn.
	// UFortRegenHealthSet inherits from UFortHealthSet and matches FFortPlayerAttributeSets::HealthSet.
	UFortRegenHealthSet* HealthSet = const_cast<UFortRegenHealthSet*>(ASC->AddSet<UFortRegenHealthSet>());
	if (HealthSet)
	{
		PlayerState->AttributeSets.HealthSet = HealthSet;
		Pawn->HealthSet = HealthSet;
		Pawn->bHealthSynced = 1;

		// Initialize default health/shield values if uninitialized (<= 0).
		if (HealthSet->MaxHealth.GetCurrentValue() <= 0.0f)
		{
			HealthSet->Health.SetBaseValue(100.0f);
			HealthSet->Health.SetCurrentValue(100.0f);
			HealthSet->MaxHealth.SetBaseValue(100.0f);
			HealthSet->MaxHealth.SetCurrentValue(100.0f);

			HealthSet->CurrentShield.SetBaseValue(0.0f);
			HealthSet->CurrentShield.SetCurrentValue(0.0f);
			HealthSet->Shield.SetBaseValue(100.0f);
			HealthSet->Shield.SetCurrentValue(100.0f);

			HealthSet->SignalInStorm.SetBaseValue(100.0f);
			HealthSet->SignalInStorm.SetCurrentValue(100.0f);
			HealthSet->MaxSignalInStorm.SetBaseValue(100.0f);
			HealthSet->MaxSignalInStorm.SetCurrentValue(100.0f);

			Pawn->SetMaxHealth(100.0f);
			Pawn->SetHealth(100.0f);
			Pawn->SetMaxShield(100.0f);
			Pawn->SetShield(0.0f);

			UE_LOG(LogFortHealth, Display, TEXT("Health: Initialized HealthSet for %s (Health=100/100, Shield=0/100)"), *Pawn->GetName());
		}
	}

	// Ensure combat and movement attribute sets exist on PlayerState/ASC.
	UFortDamageSet* DamageSet = const_cast<UFortDamageSet*>(ASC->AddSet<UFortDamageSet>());
	if (DamageSet)
	{
		PlayerState->AttributeSets.DamageSet = DamageSet;
		Pawn->DamageSet = DamageSet;
	}

	UFortControlResistanceSet* ControlSet = const_cast<UFortControlResistanceSet*>(ASC->AddSet<UFortControlResistanceSet>());
	if (ControlSet)
	{
		PlayerState->AttributeSets.ControlResistanceSet = ControlSet;
	}

	UFortMovementSet* MovementSet = const_cast<UFortMovementSet*>(ASC->AddSet<UFortMovementSet>());
	if (MovementSet)
	{
		PlayerState->AttributeSets.MovementSet = MovementSet;
	}

	UFortAdvancedMovementSet* AdvMovementSet = const_cast<UFortAdvancedMovementSet*>(ASC->AddSet<UFortAdvancedMovementSet>());
	if (AdvMovementSet)
	{
		PlayerState->AttributeSets.AdvancedMovementSet = AdvMovementSet;
	}

	// Initialize replicated movement speeds for the client-side proxy.
	if (AFortPlayerPawnAthena* AthenaPawn = Cast<AFortPlayerPawnAthena>(Pawn))
	{
		AthenaPawn->AttributeReplicationProxy.WalkSpeed = 400.0f;
		AthenaPawn->AttributeReplicationProxy.RunSpeed = 500.0f;
		AthenaPawn->AttributeReplicationProxy.SprintSpeed = 900.0f;
		AthenaPawn->AttributeReplicationProxy.FlySpeed = 600.0f;
		AthenaPawn->AttributeReplicationProxy.CrouchedRunSpeed = 300.0f;
		AthenaPawn->AttributeReplicationProxy.CrouchedSprintSpeed = 350.0f;
		AthenaPawn->OnRep_AttributeProxy();
	}

	// 4. Bind health change listener on ASC to automatically catch health depletion.
	if (!GetBoundASCs().Contains(ASC))
	{
		GetBoundASCs().Add(ASC);
		static FProperty* HealthProperty = FindFProperty<FProperty>(UFortHealthSet::StaticClass(), FName(TEXT("Health")));
		if (HealthProperty)
		{
			TWeakObjectPtr<AFortPawn> WeakPawn(Pawn);
			TWeakObjectPtr<AFortPlayerControllerAthena> WeakPC(Controller);
			ASC->GetGameplayAttributeValueChangeDelegate(FGameplayAttribute(HealthProperty)).AddLambda(
				[WeakPawn, WeakPC](const FOnAttributeChangeData& Data)
				{
					if (Data.NewValue <= 0.0f)
					{
						AFortPawn* P = WeakPawn.Get();
						if (P && !P->bIsDying)
						{
							FGameplayTagContainer DeathTags;
							AController* Instigator = nullptr;
							AActor* Causer = nullptr;
							if (Data.GEModData && Data.GEModData->EffectSpec.GetContext().IsValid())
							{
								{
									AActor* InstigatorActor = Data.GEModData->EffectSpec.GetContext().GetOriginalInstigator();
									Instigator = Cast<AController>(InstigatorActor);
									if (!Instigator)
									{
										if (APawn* InstigatorPawn = Cast<APawn>(InstigatorActor)) { Instigator = InstigatorPawn->GetController(); }
									}
								}
								Causer = Data.GEModData->EffectSpec.GetContext().GetEffectCauser();
								DeathTags = *Data.GEModData->EffectSpec.CapturedTargetTags.GetAggregatedTags();
							}
							else
							{
								// Storm or direct damage stand-in fallback tag:
								DeathTags.AddTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x360));
							}
							FortAthenaHealth::HandlePlayerEliminated(P, DeathTags, Instigator, Causer);
						}
					}
				});
		}
	}

	// 5. Grant default abilities and effects from GAS_AthenaPlayer once per PlayerState.
	if (!GetGrantedPlayerStates().Contains(PlayerState))
	{
		GetGrantedPlayerStates().Add(PlayerState);

		const TCHAR* const AbilitySetPath = TEXT("/Game/Abilities/Player/Generic/Traits/DefaultPlayer/GAS_AthenaPlayer.GAS_AthenaPlayer");
		UFortAbilitySet* AbilitySet = Cast<UFortAbilitySet>(StaticLoadObject(UFortAbilitySet::StaticClass(), nullptr, AbilitySetPath));
		if (AbilitySet)
		{
			UE_LOG(LogFortHealth, Display, TEXT("Health: Granting GAS_AthenaPlayer (%d abilities, %d effects) to %s"),
				AbilitySet->GameplayAbilities.Num(), AbilitySet->GrantedGameplayEffects.Num(), *PlayerState->GetName());

			for (const TSubclassOf<UFortGameplayAbility>& AbilityClass : AbilitySet->GameplayAbilities)
			{
				if (AbilityClass)
				{
					ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, PlayerState));
				}
			}

			for (const FGameplayEffectApplicationInfoHard& EffectInfo : AbilitySet->GrantedGameplayEffects)
			{
				if (EffectInfo.GameplayEffect)
				{
					FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
						EffectInfo.GameplayEffect, EffectInfo.Level > 0.0f ? EffectInfo.Level : 1.0f, ASC->MakeEffectContext());
					if (SpecHandle.IsValid())
					{
						ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					}
				}
			}
		}
		else
		{
			UE_LOG(LogFortHealth, Warning, TEXT("Health: GAS_AthenaPlayer not loaded (asset not yet staged: %s)"), AbilitySetPath);
			// stand-in: grant core native abilities so basic jump/sprint work even when the cooked Blueprint asset is not staged
			if (UClass* JumpClass = UFortGameplayAbility_Jump::StaticClass())
			{
				ASC->GiveAbility(FGameplayAbilitySpec(JumpClass, 1, INDEX_NONE, PlayerState));
			}
			if (UClass* SprintClass = UFortGameplayAbility_Sprint::StaticClass())
			{
				ASC->GiveAbility(FGameplayAbilitySpec(SprintClass, 1, INDEX_NONE, PlayerState));
			}
		}
	}

	// 6. Track alive players list and synchronize GameState->PlayersLeft.
	TArray<TWeakObjectPtr<AFortPlayerControllerAthena>>& Alive = GetAlivePlayers();
	Alive.RemoveAll([](const TWeakObjectPtr<AFortPlayerControllerAthena>& P) { return !P.IsValid(); });
	if (!Alive.Contains(Controller))
	{
		Alive.Add(Controller);
	}

	if (AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(GameMode->GameState))
	{
		GameState->PlayersLeft = Alive.Num();
		GameState->OnRep_PlayersLeft();
	}
}

void FortAthenaHealth::ApplyDamage(AFortPawn* VictimPawn, float DamageAmount, const FGameplayTagContainer& DamageTags,
                                  AController* InstigatedBy, AActor* DamageCauser, bool bIgnoreShield)
{
	if (!IsEnabled() || !VictimPawn || DamageAmount <= 0.0f || VictimPawn->bIsDying)
	{
		return;
	}

	float ShieldDamage = 0.0f;
	float HealthDamage = DamageAmount;
	const float CurrentShield = VictimPawn->GetShield();

	// Shield absorbs damage first unless ignored (e.g. storm damage or fall damage).
	if (!bIgnoreShield && CurrentShield > 0.0f)
	{
		if (DamageAmount <= CurrentShield)
		{
			ShieldDamage = DamageAmount;
			HealthDamage = 0.0f;
			VictimPawn->SetShield(CurrentShield - DamageAmount);
		}
		else
		{
			ShieldDamage = CurrentShield;
			HealthDamage = DamageAmount - CurrentShield;
			VictimPawn->SetShield(0.0f);
		}
	}

	const float OldHealth = VictimPawn->GetHealth();
	float NewHealth = OldHealth;
	if (HealthDamage > 0.0f)
	{
		NewHealth = FMath::Max(0.0f, OldHealth - HealthDamage);
		VictimPawn->SetHealth(NewHealth);
	}

	// Multicast damage cues to client (hit numbers, hit markers, shield shatter).
	FAthenaBatchedDamageGameplayCues_Shared SharedCues{};
	SharedCues.Location = FVector_NetQuantize10(VictimPawn->GetActorLocation());
	SharedCues.Normal = FVector_NetQuantizeNormal(FVector::UpVector);
	SharedCues.Magnitude = DamageAmount;
	SharedCues.bIsShield = (ShieldDamage > 0.0f);
	SharedCues.bIsShieldDestroyed = (ShieldDamage > 0.0f && VictimPawn->GetShield() <= 0.0f);
	SharedCues.bIsFatal = (NewHealth <= 0.0f);
	SharedCues.bIsValid = true;

	FAthenaBatchedDamageGameplayCues_NonShared NonSharedCues{};
	NonSharedCues.HitActor = VictimPawn;

	VictimPawn->NetMulticast_Athena_BatchedDamageCues(SharedCues, NonSharedCues);

	UE_LOG(LogFortHealth, Display, TEXT("Health: Damage %.1f applied to %s (Shield: -%.1f, Health: -%.1f) -> Health: %.1f, Shield: %.1f"),
		DamageAmount, *VictimPawn->GetName(), ShieldDamage, HealthDamage, NewHealth, VictimPawn->GetShield());

	if (NewHealth <= 0.0f && !VictimPawn->bIsDying)
	{
		HandlePlayerEliminated(VictimPawn, DamageTags, InstigatedBy, DamageCauser);
	}
}

void FortAthenaHealth::HandlePlayerEliminated(AFortPawn* VictimPawn, const FGameplayTagContainer& DamageTags,
                                             AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsEnabled() || !VictimPawn || VictimPawn->bIsDying)
	{
		return;
	}

	// Mark dying immediately to prevent re-entrant calls.
	VictimPawn->bIsDying = 1;
	VictimPawn->bWasDBNOOnDeath = VictimPawn->bIsDBNO;
	if (UCharacterMovementComponent* MoveComp = VictimPawn->GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}
	VictimPawn->SetCanBeDamaged(false);
	VictimPawn->OnDied.Broadcast(VictimPawn, 0.0f, InstigatedBy, DamageCauser, VictimPawn->GetActorLocation(), nullptr, NAME_None, FVector::ZeroVector);

	AFortPlayerControllerAthena* VictimPC = Cast<AFortPlayerControllerAthena>(VictimPawn->GetController());
	AFortPlayerStateAthena* VictimState = Cast<AFortPlayerStateAthena>(VictimPawn->GetPlayerState());
	if (!VictimState && VictimPC)
	{
		VictimState = Cast<AFortPlayerStateAthena>(VictimPC->PlayerState);
	}
	if (!VictimPC && VictimState)
	{
		VictimPC = Cast<AFortPlayerControllerAthena>(VictimState->GetOwner());
	}

	AFortPlayerStateAthena* KillerState = InstigatedBy ? Cast<AFortPlayerStateAthena>(InstigatedBy->PlayerState) : nullptr;
	AFortPawn* KillerPawn = InstigatedBy ? Cast<AFortPawn>(InstigatedBy->GetPawn()) : nullptr;
	UFortWeaponItemDefinition* KillerWeapon = (KillerPawn && KillerPawn->CurrentWeapon) ? KillerPawn->CurrentWeapon->WeaponData : nullptr;

	// 1. Build FFortPlayerDeathReport (D6 §2 step 1).
	FFortPlayerDeathReport Report{};
	Report.ServerTimeForRespawn = -1.0f;
	Report.ServerTimeForResurrect = -1.0f;
	Report.LethalDamage = 100.0f;
	Report.KillerPlayerState = KillerState;
	Report.KillerPawn = KillerPawn;
	Report.KillerHealthPercent = KillerPawn ? KillerPawn->GetHealthPercent() : 0.0f;
	Report.KillerShieldPercent = KillerPawn ? KillerPawn->GetShieldPercent() : 0.0f;
	Report.KillerWeapon = KillerWeapon;
	Report.DamageCauser = DamageCauser;
	Report.bDroppedBackpack = 1;
	Report.bNotifyUI = 1;
	Report.Tags = DamageTags;
	Report.ViewLocationAtTimeOfDeath = VictimPawn->GetActorLocation();
	Report.ViewRotationAtTimeOfDeath = VictimPC ? VictimPC->GetControlRotation() : FRotator::ZeroRotator;

	// 2. Populate and replicate victim DeathInfo (D6 §2 step 2).
	if (VictimState)
	{
		FDeathInfo Info{};
		Info.FinisherOrDowner = KillerState ? static_cast<AActor*>(KillerState) : (DamageCauser ? DamageCauser : static_cast<AActor*>(VictimPawn));
		Info.Downer = nullptr;
		Info.bDBNO = false;
		Info.DeathCause = AFortPlayerStateAthena::ToDeathCause(const_cast<FGameplayTagContainer&>(DamageTags), false);
		Info.DeathClassSlot = 0;
		Info.Distance = (KillerPawn && VictimPawn) ? FVector::Dist(KillerPawn->GetActorLocation(), VictimPawn->GetActorLocation()) : 0.0f;
		Info.DeathLocation = VictimPawn->GetActorLocation();
		Info.bInitialized = true;
		Info.DeathTags = DamageTags;

		VictimState->DeathInfo = Info;
		VictimState->PawnDeathLocation = Info.DeathLocation;
		VictimState->OnRep_DeathInfo();
	}

	// 3. Update killer scores and feed (D6 §2 step 3).
	if (KillerState && KillerState != VictimState)
	{
		KillerState->KillScore += 1;
		KillerState->TeamKillScore += 1;
		KillerState->OnRep_Kills();
		KillerState->OnRep_TeamKillScore();
		KillerState->ClientReportKill(VictimState);
		UE_LOG(LogFortHealth, Display, TEXT("Health: Killer %s elimination score now %d"), *KillerState->GetName(), KillerState->KillScore);
	}

	// 4. Notify victim controller (D6 §2 step 4).
	if (VictimPC)
	{
		VictimPC->ClientOnPawnDied(Report);
		VictimPC->ClientSetDeathReport(Report);
	}

	// 5. Update alive players and GameState->PlayersLeft (R3 §2).
	UWorld* World = VictimPawn->GetWorld();
	AFortGameStateAthena* GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
	const int32 OldPlayersLeft = GameState ? GameState->PlayersLeft : GetAlivePlayerCount();

	TArray<TWeakObjectPtr<AFortPlayerControllerAthena>>& Alive = GetAlivePlayers();
	if (VictimPC)
	{
		Alive.Remove(VictimPC);
	}
	Alive.RemoveAll([](const TWeakObjectPtr<AFortPlayerControllerAthena>& P) { return !P.IsValid(); });

	const int32 NewPlayersLeft = Alive.Num();
	if (GameState)
	{
		GameState->PlayersLeft = NewPlayersLeft;
		GameState->OnRep_PlayersLeft();
	}
	UE_LOG(LogFortHealth, Display, TEXT("Health: Player %s removed from alive list. PlayersLeft is now %d (was %d)"),
		VictimPC ? *VictimPC->GetName() : TEXT("Unknown"), NewPlayersLeft, OldPlayersLeft);

	// 6. Placement and end-match stats for victim (D6 §2 step 7).
	const int32 Place = NewPlayersLeft + 1;
	if (VictimState)
	{
		VictimState->Place = Place;
		VictimState->OnRep_Place();
		if (GameState)
		{
			VictimState->SecondsAlive = static_cast<int32>(GameState->GetServerWorldTimeSeconds());
			VictimState->NumMinutesAlive = VictimState->SecondsAlive / 60;
			VictimState->OnRep_NumMinutesAlive();
		}
	}

	if (VictimPC)
	{
		// stand-in: fill team and match stats to drive "You placed #N" client UI
		FAthenaMatchTeamStats TeamStats{};
		TeamStats.Place = Place;
		TeamStats.TotalPlayers = FMath::Max(OldPlayersLeft, Place);
		VictimPC->ClientSendTeamStatsForPlayer(TeamStats);

		FAthenaMatchStats MatchStats{};
		MatchStats.StatBucket = TEXT("Default");
		VictimPC->ClientSendMatchStatsForPlayer(MatchStats);

		FAthenaRewardResult RewardResult{};
		VictimPC->ClientSendEndBattleRoyaleMatchForPlayer(true, RewardResult);
		UE_LOG(LogFortHealth, Display, TEXT("Health: Sent end match stats to %s (Place #%d of %d)"),
			*VictimPC->GetName(), Place, TeamStats.TotalPlayers);
	}

	// 7. Check Win Condition (R3 §3 step 4) and Victory Royale (R3 §4 StartEndGamePhase).
	if (GameState && (GameState->GamePhase == EAthenaGamePhase::Aircraft || GameState->GamePhase == EAthenaGamePhase::SafeZones))
	{
		if (NewPlayersLeft == 1 && OldPlayersLeft >= 2 && Alive.Num() == 1)
		{
			AFortPlayerControllerAthena* WinnerPC = Alive[0].Get();
			if (WinnerPC)
			{
				AFortPlayerStateAthena* WinnerState = Cast<AFortPlayerStateAthena>(WinnerPC->PlayerState);
				AFortPawn* FinisherPawn = Cast<AFortPawn>(WinnerPC->GetPawn());
				const EDeathCause WinCause = EDeathCause::WonMatch;

				UE_LOG(LogFortHealth, Display, TEXT("Health: %s is the last player alive! Winner of the match (Victory Royale)!"), *WinnerPC->GetName());

				if (WinnerState)
				{
					WinnerState->Place = 1;
					WinnerState->OnRep_Place();
				}
				GameState->WinningPlayerState = WinnerState;
				GameState->OnRep_WinningPlayerState();

				WinnerPC->ClientNotifyWon(FinisherPawn, nullptr, WinCause);

				FAthenaMatchTeamStats WinnerTeamStats{};
				WinnerTeamStats.Place = 1;
				WinnerTeamStats.TotalPlayers = FMath::Max(OldPlayersLeft, 1);
				WinnerPC->ClientSendTeamStatsForPlayer(WinnerTeamStats);

				FAthenaMatchStats WinnerMatchStats{};
				WinnerMatchStats.StatBucket = TEXT("Default");
				WinnerPC->ClientSendMatchStatsForPlayer(WinnerMatchStats);

				FAthenaRewardResult WinnerRewardResult{};
				WinnerPC->ClientSendEndBattleRoyaleMatchForPlayer(true, WinnerRewardResult);

				const EAthenaGamePhase OldPhase = GameState->GamePhase;
				GameState->GamePhase = EAthenaGamePhase::EndGame;
				GameState->OnRep_GamePhase(OldPhase);
				UE_LOG(LogFortHealth, Display, TEXT("Health: GamePhase transitioned to EndGame (5)"));
			}
		}
	}
}

void FortAthenaHealth::InstallRpcOverrides()
{
	if (!IsEnabled())
	{
		return;
	}
	static bool bInstalled = false;
	if (bInstalled)
	{
		return;
	}

	if (UFunction* Function = AFortPlayerPawnAthena::StaticClass()->FindFunctionByName(TEXT("ServerSuicide")))
	{
		Function->SetNativeFunc(&ExecServerSuicideOverride);
		UE_LOG(LogFortHealth, Display, TEXT("Health: ServerSuicide rebound to the re-created body"));
	}
	else
	{
		UE_LOG(LogFortHealth, Warning, TEXT("Health: AFortPlayerPawnAthena::ServerSuicide UFunction not found"));
	}

	if (UFunction* Function = AFortPlayerPawn::StaticClass()->FindFunctionByName(TEXT("ServerResetAbilitySystemComponent")))
	{
		Function->SetNativeFunc(&ExecServerResetAbilitySystemComponentOverride);
		UE_LOG(LogFortHealth, Display, TEXT("Health: AFortPlayerPawn::ServerResetAbilitySystemComponent rebound to the re-created body"));
	}
	else
	{
		UE_LOG(LogFortHealth, Warning, TEXT("Health: AFortPlayerPawn::ServerResetAbilitySystemComponent UFunction not found"));
	}

	bInstalled = true;
}

// Global-namespace wrappers for behaviour snippets (.inc bodies).
void FortAthenaHealth_ApplyDamage(AFortPawn* VictimPawn, float DamageAmount, const FGameplayTagContainer& DamageTags,
                                 AController* InstigatedBy, AActor* DamageCauser, bool bIgnoreShield)
{
	FortAthenaHealth::ApplyDamage(VictimPawn, DamageAmount, DamageTags, InstigatedBy, DamageCauser, bIgnoreShield);
}

void FortAthenaHealth_HandlePlayerEliminated(AFortPawn* VictimPawn, const FGameplayTagContainer& DamageTags,
                                            AController* InstigatedBy, AActor* DamageCauser)
{
	FortAthenaHealth::HandlePlayerEliminated(VictimPawn, DamageTags, InstigatedBy, DamageCauser);
}
