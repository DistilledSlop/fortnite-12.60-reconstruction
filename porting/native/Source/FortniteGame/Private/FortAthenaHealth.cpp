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
#include "FortAthenaEndGame.h"
#include "UFortAssetManager.h"
#include "Engine/Engine.h"
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
#include "TimerManager.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortHealth, Log, All);

namespace
{
	TWeakObjectPtr<AFortGameModeAthena> RosterMode;
	int32 MatchParticipants = 0;
	float ParticipationStart = -1.0f;
	bool bRosterFrozen = false;
	TSet<TWeakObjectPtr<AFortPlayerControllerAthena>> FinalizedPlayers;

	// Tracks active, alive player controllers in the match.
	TArray<TWeakObjectPtr<AFortPlayerControllerAthena>>& GetAlivePlayers()
	{
		static TArray<TWeakObjectPtr<AFortPlayerControllerAthena>> List;
		return List;
	}

	void ResetRosterFor(AFortGameModeAthena* Mode)
	{
		if (RosterMode.Get() != Mode)
		{
			RosterMode = Mode;
			GetAlivePlayers().Reset();
			FinalizedPlayers.Reset();
			MatchParticipants = 0;
			ParticipationStart = -1.0f;
			bRosterFrozen = false;
		}
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

void FortAthenaHealth::BeginMatch(AFortGameModeAthena* GameMode)
{
	if (!IsEnabled() || !GameMode) return;
	ResetRosterFor(GameMode);
	if (bRosterFrozen) return;
	AFortGameStateAthena* GS = GameMode->GetGameState<AFortGameStateAthena>();
	if (!GS) return;
	MatchParticipants = GetAlivePlayerCount();
	ParticipationStart = GS->GetServerWorldTimeSeconds();
	bRosterFrozen = true;
}

AFortPlayerControllerAthena* FortAthenaHealth::GetSoleSurvivor(AFortGameModeAthena* GameMode)
{
	if (!GameMode || RosterMode.Get() != GameMode || GetAlivePlayerCount() != 1) return nullptr;
	return GetAlivePlayers()[0].Get();
}

void FortAthenaHealth::FinalizeSurvival(AFortGameModeAthena* GameMode, AFortPlayerStateAthena* PS)
{
	AFortGameStateAthena* GS = GameMode ? GameMode->GetGameState<AFortGameStateAthena>() : nullptr;
	if (!PS || !GS || RosterMode.Get() != GameMode || ParticipationStart < 0.0f) return;
	PS->SecondsAlive = FMath::Max(0, FMath::FloorToInt(GS->GetServerWorldTimeSeconds() - ParticipationStart));
	PS->NumMinutesAlive = PS->SecondsAlive / 60;
	PS->OnRep_NumMinutesAlive();
}

void FortAthenaHealth::SendPartialTeamStats(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Controller, int32 Place)
{
	if (!GameMode || RosterMode.Get() != GameMode || !Controller || !bRosterFrozen) return;
	// Detailed stats and rewards require recovered semantics.
	FAthenaMatchTeamStats TeamStats{};
	TeamStats.Place = Place;
	TeamStats.TotalPlayers = MatchParticipants;
	Controller->ClientSendTeamStatsForPlayer(TeamStats);
	UE_LOG(LogFortHealth, Display, TEXT("Health: Sent partial team stats to %s (Place #%d of %d)"),
		*Controller->GetName(), Place, TeamStats.TotalPlayers);
}

namespace
{
	// stand-in for AFortPlayerState's native constructor: the original creates the ability system component and its
	// attribute sets as default subobjects of the PlayerState with fixed names (AbilitySystemComponent, HealthSet,
	// DamageSet, ...; generated/reflection/native_default_subobjects.json, FortPlayerStateEndless). Our generated class has
	// no constructor, so they are created here under the same names and flagged RF_DefaultSubObject. That makes them
	// stably named for networking, so the client maps them onto its own default subobjects instead of spawning extra
	// copies (run 27: the client pawn never got a HealthSet and AFortPawn::GetHealth 0x14231c4c0 fell back to 1).
	template <class T>
	T* AddNamedSet(UAbilitySystemComponent* ASC, AActor* Owner, const TCHAR* Name)
	{
		if (const T* Existing = ASC->GetSet<T>())
		{
			return const_cast<T*>(Existing);
		}
		T* Set = NewObject<T>(Owner, T::StaticClass(), FName(Name), RF_DefaultSubObject);
		ASC->AddAttributeSetSubobject(Set);
		return Set;
	}
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
		ASC = NewObject<UFortAbilitySystemComponent>(PlayerState, TEXT("AbilitySystemComponent"), RF_DefaultSubObject);  // see AddNamedSet
		ASC->SetNetAddressable();
		ASC->SetIsReplicated(true);
		ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
		PlayerState->AbilitySystemComponent = ASC;
		PlayerState->AddOwnedComponent(ASC);
		ASC->RegisterComponent();
		UE_LOG(LogFortHealth, Display, TEXT("Health: Created and registered AbilitySystemComponent on %s"), *PlayerState->GetName());
	}
	// stand-in: the vanilla client reads gameplay-cue RPCs (NetMulticast_InvokeGameplayCue*) with Fortnite's own
	// FGameplayEffectContext / FGameplayCueParameters serialisation; our stock-engine layout fails with "ReceivePropertiesForRPC -
	// Mismatch read" and the client closes the connection (run 43). Suppress cue invocation on the server until that is ported.
	ASC->bSuppressGameplayCues = true;

	// 2. Initialize AbilityActorInfo: Owner is PlayerState, Avatar is Pawn.
	ASC->InitAbilityActorInfo(PlayerState, Pawn);
	Pawn->AbilitySystemComponent = ASC;
	Pawn->bInitAbilitySystemComponentFromPlayerState = 1;

	// 3. Ensure attribute sets exist on PlayerState/ASC and are mirrored on Pawn.
	// UFortRegenHealthSet inherits from UFortHealthSet and matches FFortPlayerAttributeSets::HealthSet.
	UFortRegenHealthSet* HealthSet = AddNamedSet<UFortRegenHealthSet>(ASC, PlayerState, TEXT("HealthSet"));
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
	UFortDamageSet* DamageSet = AddNamedSet<UFortDamageSet>(ASC, PlayerState, TEXT("DamageSet"));
	if (DamageSet)
	{
		PlayerState->AttributeSets.DamageSet = DamageSet;
		Pawn->DamageSet = DamageSet;
	}

	UFortControlResistanceSet* ControlSet = AddNamedSet<UFortControlResistanceSet>(ASC, PlayerState, TEXT("ControlResistanceSet"));
	if (ControlSet)
	{
		PlayerState->AttributeSets.ControlResistanceSet = ControlSet;
	}

	UFortMovementSet* MovementSet = AddNamedSet<UFortMovementSet>(ASC, PlayerState, TEXT("MovementSet"));
	if (MovementSet)
	{
		PlayerState->AttributeSets.MovementSet = MovementSet;
	}

	UFortAdvancedMovementSet* AdvMovementSet = AddNamedSet<UFortAdvancedMovementSet>(ASC, PlayerState, TEXT("AdvancedMovementSet"));
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

	// 4. The ASC belongs to the PlayerState and survives avatar replacement.
	// Prune dead component keys; the delegate and its shared serial die with the ASC.
	for (auto It = GetBoundASCs().CreateIterator(); It; ++It)
	{
		if (!It->IsValid()) { It.RemoveCurrent(); }
	}
	if (!GetBoundASCs().Contains(ASC))
	{
		static FProperty* HealthProperty = FindFProperty<FProperty>(UFortHealthSet::StaticClass(), FName(TEXT("Health")));
		if (HealthProperty)
		{
			GetBoundASCs().Add(ASC);
			TWeakObjectPtr<UAbilitySystemComponent> WeakASC(ASC);
			TSharedRef<uint64> ChangeSerial = MakeShared<uint64>(0);
			ASC->GetGameplayAttributeValueChangeDelegate(FGameplayAttribute(HealthProperty)).AddLambda(
				[WeakASC, ChangeSerial](const FOnAttributeChangeData& Data)
				{
					const uint64 Serial = ++(*ChangeSerial);
					UAbilitySystemComponent* Component = WeakASC.Get();
					AFortPawn* PawnAtChange = Component ? Cast<AFortPawn>(Component->GetAvatarActor()) : nullptr;
					UWorld* World = PawnAtChange ? PawnAtChange->GetWorld() : nullptr;
					if (Data.NewValue > 0.0f || !World) { return; }

					// GEModData is stack-owned by GAS. Copy attribution now; it is not
					// necessarily present for changes made by PostGameplayEffectExecute.
					FGameplayTagContainer DeathTags;
					TWeakObjectPtr<AController> WeakInstigator;
					TWeakObjectPtr<AActor> WeakCauser;
					if (Data.GEModData)
					{
						const FGameplayEffectSpec& Spec = Data.GEModData->EffectSpec;
						Spec.GetAllAssetTags(DeathTags);
						DeathTags.AppendTags(*Spec.CapturedSourceTags.GetAggregatedTags());
						if (Spec.GetContext().IsValid())
						{
							AActor* Source = Spec.GetContext().GetOriginalInstigator();
							AController* SourceController = Cast<AController>(Source);
							if (!SourceController)
							{
								if (APawn* SourcePawn = Cast<APawn>(Source)) { SourceController = SourcePawn->GetController(); }
							}
							WeakInstigator = SourceController;
							WeakCauser = Spec.GetContext().GetEffectCauser();
						}
					}
					TWeakObjectPtr<AFortPawn> WeakPawn(PawnAtChange);
					// The health delegate can fire inside GAS before the HealthSet's
					// synchronous DBNO interception. Recheck after that stack returns.
					World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda(
						[WeakASC, WeakPawn, WeakInstigator, WeakCauser, ChangeSerial, Serial, DeathTags]()
						{
							UAbilitySystemComponent* CurrentASC = WeakASC.Get();
							AFortPawn* CurrentPawn = WeakPawn.Get();
							if (*ChangeSerial != Serial || !CurrentASC || !CurrentPawn ||
								CurrentASC->GetAvatarActor() != CurrentPawn || CurrentPawn->bIsDying ||
								CurrentPawn->bIsDBNO || CurrentPawn->GetHealth() > 0.0f) { return; }
							FortAthenaHealth::HandlePlayerEliminated(CurrentPawn, DeathTags, WeakInstigator.Get(), WeakCauser.Get());
						}));
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
	ResetRosterFor(GameMode);
	TArray<TWeakObjectPtr<AFortPlayerControllerAthena>>& Alive = GetAlivePlayers();
	Alive.RemoveAll([](const TWeakObjectPtr<AFortPlayerControllerAthena>& P) { return !P.IsValid(); });
	if (!bRosterFrozen && !FinalizedPlayers.Contains(Controller) && !Alive.Contains(Controller))
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

	AFortPlayerControllerAthena* VictimPC = Cast<AFortPlayerControllerAthena>(VictimPawn->GetController());
	AFortPlayerStateAthena* VictimState = Cast<AFortPlayerStateAthena>(VictimPawn->GetPlayerState());
	if (!VictimState && VictimPC)
		VictimState = Cast<AFortPlayerStateAthena>(VictimPC->PlayerState);
	if (!VictimPC && VictimState)
		VictimPC = Cast<AFortPlayerControllerAthena>(VictimState->GetOwner());
	UWorld* World = VictimPawn->GetWorld();
	AFortGameStateAthena* GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
	AFortGameModeAthena* GameMode = World ? Cast<AFortGameModeAthena>(World->GetAuthGameMode()) : nullptr;
	TArray<TWeakObjectPtr<AFortPlayerControllerAthena>>& Alive = GetAlivePlayers();
	if (!GameMode || RosterMode.Get() != GameMode || !VictimPC || !Alive.Contains(VictimPC))
	{
		UE_LOG(LogFortHealth, Warning, TEXT("Health: eliminated player absent from roster; cannot finalize placement"));
		return;
	}
	const int32 OldPlayersLeft = GetAlivePlayerCount();

	// Reserve the removal before callbacks can re-enter the death flow.
	VictimPawn->bIsDying = 1;
	FinalizedPlayers.Add(VictimPC);
	Alive.Remove(VictimPC);
	const int32 NewPlayersLeft = Alive.Num();
	if (GameState)
	{
		GameState->PlayersLeft = NewPlayersLeft;
		GameState->OnRep_PlayersLeft();
	}
	VictimPawn->bWasDBNOOnDeath = VictimPawn->bIsDBNO;
	if (UCharacterMovementComponent* MoveComp = VictimPawn->GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}
	VictimPawn->SetCanBeDamaged(false);
	VictimPawn->OnDied.Broadcast(VictimPawn, 0.0f, InstigatedBy, DamageCauser, VictimPawn->GetActorLocation(), nullptr, NAME_None, FVector::ZeroVector);

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
		// The recovered getter is fatal without common game data. A failed startup load must remain diagnosable.
		const UFortAssetManager* Manager = GEngine ? Cast<UFortAssetManager>(GEngine->AssetManager) : nullptr;
		if (Manager && Manager->GameDataCommon)
			Info.DeathCause = AFortPlayerStateAthena::ToDeathCause(const_cast<FGameplayTagContainer&>(DamageTags), false);
		else
		{
			const FGameplayTag StormTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x360);
			Info.DeathCause = StormTag.IsValid() && DamageTags.HasTag(StormTag)
				? EDeathCause::OutsideSafeZone : EDeathCause::Unspecified;
		}
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

	// 5. The roster removal above is already visible to re-entrant callbacks.
	UE_LOG(LogFortHealth, Display, TEXT("Health: Player %s removed from alive list. PlayersLeft is now %d (was %d)"),
		VictimPC ? *VictimPC->GetName() : TEXT("Unknown"), NewPlayersLeft, OldPlayersLeft);

	// 6. Placement and end-match stats for victim (D6 §2 step 7).
	const int32 Place = NewPlayersLeft + 1;
	if (VictimState)
	{
		VictimState->Place = Place;
		VictimState->OnRep_Place();
		FinalizeSurvival(GameMode, VictimState);
	}

	SendPartialTeamStats(GameMode, VictimPC, Place);

	// Single winner boundary: pass the actual eliminating event and roster snapshot.
	FortAthenaEndGame::OnPlayerEliminated(GameMode, OldPlayersLeft, NewPlayersLeft,
		GetSoleSurvivor(GameMode), KillerPawn, KillerWeapon,
		VictimState ? VictimState->DeathInfo.DeathCause : EDeathCause::Unspecified);
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
