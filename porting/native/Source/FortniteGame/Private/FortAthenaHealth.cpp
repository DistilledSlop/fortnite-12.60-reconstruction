// Server-side health, GAS (Ability System Component), attribute sets, damage, and death sequence
// for the original 12.60 client (WP G8-Health, https://github.com/DistilledSlop/fortnite-12.60-reconstruction/wiki/Server-First-Scope-Mockup).
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
#include "ServerDamageExecution.h"
#include "UFortPlaylistAthena.h"
#include "FortAthenaLootTransfer.h"
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
#include "UFortAbilitySystemComponentAthena.h"
#include "UFortCharacterAttrSet.h"
#include "UFortPlayerAttrSet.h"
#include "UFortHomebaseSet.h"
#include "UFortHero.h"
#include "UFortHeroType.h"
#include "UFortRegisteredPlayerInfo.h"
#include "FortAttributeSetInitter.h"
#include "FortPlayerStatFallback.h"
#include "AbilitySystemGlobals.h"
#include "GameplayEffectAggregator.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
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
#include "Components/CapsuleComponent.h"
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

namespace
{
	void ReleaseCarriedPlayer(AFortPlayerPawn* Carrier, AFortPlayerPawn* Target, bool bThrow)
	{
		if (IsValid(Carrier))
		{
			Carrier->GetCapsuleComponent()->IgnoreActorWhenMoving(Target, false);
			Carrier->DBNOHoistee = nullptr;
			Carrier->bIsDBNOCarrying = false;
			Carrier->ForceNetUpdate();
		}
		if (!IsValid(Target))
		{
			return;
		}

		Target->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		Target->DBNOHoisterData.DBNOHoister = nullptr;
		Target->DBNOHoisterData.DBNOCarryEvent = bThrow ? EFortDBNOCarryEvent::Thrown : EFortDBNOCarryEvent::Dropped;
		Target->bIsBeingDBNOCarried = false;
		if (!Target->bIsDying)
		{
			Target->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			if (IsValid(Carrier))
			{
				const FVector Forward = Carrier->GetActorForwardVector().GetSafeNormal2D();
				const float Distance = Carrier->DropCarriedPlayerForwardOffset > 0.0f ? Carrier->DropCarriedPlayerForwardOffset : 100.0f;
				Target->SetActorLocation(Target->GetActorLocation() + Forward * Distance, true);
				if (!bThrow)
				{
					FVector DropLocation = Target->GetActorLocation();
					DropLocation.Z = Carrier->GetActorLocation().Z + Carrier->DropCarriedPlayerHeightOffset;
					Target->SetActorLocation(DropLocation, true);
				}
				if (bThrow)
				{
					const float Speed = Carrier->ThrowCarriedPlayerStrengthXY > 0.0f ? Carrier->ThrowCarriedPlayerStrengthXY : 750.0f;
					const float Lift = Carrier->ThrowCarriedPlayerStrengthZ > 0.0f ? Carrier->ThrowCarriedPlayerStrengthZ : 750.0f;
					Target->LaunchCharacter(Forward * Speed + FVector::UpVector * Lift, true, true);
				}
			}
		}
		Target->GetCapsuleComponent()->IgnoreActorWhenMoving(Carrier, false);
		Target->ForceNetUpdate();
	}

	void ReleaseCarryConnections(AFortPawn* Pawn)
	{
		if (AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(Pawn))
		{
			if (Player->DBNOHoistee)
			{
				ReleaseCarriedPlayer(Player, Player->DBNOHoistee, false);
			}
			if (Player->bIsBeingDBNOCarried)
			{
				ReleaseCarriedPlayer(Player->DBNOHoisterData.DBNOHoister, Player, false);
			}
		}
	}

	void ExecHoistDownedPlayer(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(AFortPlayerPawn, Target);
		P_FINISH;
		AFortPlayerPawn* Carrier = Cast<AFortPlayerPawn>(Context);
		AController* Controller = Carrier ? Carrier->GetController() : nullptr;
		if (Controller && Controller->GetPawn() == Carrier)
		{
			FortAthenaHealth::TryCarryDownedPlayer(Controller, Target);
		}
	}

	void ExecDropCarriedPlayer(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_FINISH;
		AFortPlayerPawn* Carrier = Cast<AFortPlayerPawn>(Context);
		if (Carrier && Carrier->HasAuthority() && Carrier->DBNOHoistee)
		{
			ReleaseCarriedPlayer(Carrier, Carrier->DBNOHoistee, false);
		}
	}

	void ExecThrowCarriedPlayer(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_FINISH;
		AFortPlayerPawn* Carrier = Cast<AFortPlayerPawn>(Context);
		if (Carrier && Carrier->HasAuthority() && !Carrier->bIsDBNO && !Carrier->bIsDying && Carrier->DBNOHoistee)
		{
			ReleaseCarriedPlayer(Carrier, Carrier->DBNOHoistee, true);
		}
	}
}

bool FortAthenaHealth::TryCarryDownedPlayer(AController* Controller, AFortPawn* Target)
{
	AFortPlayerPawnAthena* Carrier = Controller ? Cast<AFortPlayerPawnAthena>(Controller->GetPawn()) : nullptr;
	AFortPlayerPawnAthena* Downed = Cast<AFortPlayerPawnAthena>(Target);
	constexpr float CarryRange = 250.0f;
	if (!IsEnabled() || !IsValid(Carrier) || !IsValid(Downed) || Carrier == Downed ||
		!Carrier->HasAuthority() || !Carrier->bAllowDBNOCarry || Carrier->bIsDBNO || Carrier->bIsDying ||
		Carrier->GetHealth() <= 0.0f || Carrier->ItemInteractionActor || Carrier->DBNOHoistee || !Downed->bIsDBNO || Downed->bIsDying ||
		Downed->bIsBeingDBNOCarried || Downed->DBNORevivalStacking != 0 ||
		FVector::DistSquared(Carrier->GetActorLocation(), Downed->GetActorLocation()) > FMath::Square(CarryRange) ||
		!Controller->LineOfSightTo(Downed))
	{
		return false;
	}

	const AFortPlayerStateAthena* CarrierState = Cast<AFortPlayerStateAthena>(Carrier->GetPlayerState());
	const AFortPlayerStateAthena* DownedState = Cast<AFortPlayerStateAthena>(Downed->GetPlayerState());
	if (!CarrierState || !DownedState ||
		(CarrierState->TeamIndex != DownedState->TeamIndex && !Carrier->bAllowDBNOCarryEnemies))
	{
		return false;
	}

	Downed->AttachToActor(Carrier, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	if (Downed->GetAttachParentActor() != Carrier)
	{
		return false;
	}
	Downed->SetActorRelativeLocation(FVector(0.0f, 0.0f, 75.0f));
	Downed->GetCharacterMovement()->DisableMovement();
	Carrier->GetCapsuleComponent()->IgnoreActorWhenMoving(Downed, true);
	Downed->GetCapsuleComponent()->IgnoreActorWhenMoving(Carrier, true);
	Downed->DBNOHoisterData.DBNOHoister = Carrier;
	Downed->DBNOHoisterData.DBNOCarryEvent = EFortDBNOCarryEvent::PickedUp;
	Downed->bIsBeingDBNOCarried = true;
	Carrier->DBNOHoistee = Downed;
	Carrier->bIsDBNOCarrying = true;
	Carrier->ForceNetUpdate();
	Downed->ForceNetUpdate();
	return true;
}

namespace
{
	bool IsStanding(const AFortPlayerControllerAthena* Controller)
	{
		const AFortPawn* Pawn = Controller ? Cast<AFortPawn>(Controller->GetPawn()) : nullptr;
		return Pawn && !Pawn->bIsDying && !Pawn->bIsDBNO && Pawn->GetHealth() > 0.0f;
	}

	bool HasStandingTeammate(const AFortPlayerPawnAthena* Player)
	{
		AFortPlayerControllerAthena* Controller = Cast<AFortPlayerControllerAthena>(Player->GetController());
		const AFortPlayerStateAthena* PlayerState = Controller ? Cast<AFortPlayerStateAthena>(Controller->PlayerState) : nullptr;
		const auto& Alive = GetAlivePlayers();
		if (!PlayerState || !Alive.Contains(Controller))
		{
			return false;
		}

		return Alive.ContainsByPredicate([Controller, PlayerState](const auto& Entry)
		{
			const AFortPlayerControllerAthena* Teammate = Entry.Get();
			const AFortPlayerStateAthena* TeammateState = Teammate ? Cast<AFortPlayerStateAthena>(Teammate->PlayerState) : nullptr;
			return Teammate != Controller && TeammateState && TeammateState->TeamIndex == PlayerState->TeamIndex &&
				IsStanding(Teammate);
		});
	}

	bool TryKnockPlayer(AFortPawn* Pawn, const FGameplayTagContainer& DamageTags,
	                    AController* InstigatedBy, AActor* DamageCauser)
	{
		AFortPlayerPawnAthena* Player = Cast<AFortPlayerPawnAthena>(Pawn);
		UWorld* World = Player ? Player->GetWorld() : nullptr;
		AFortGameModeAthena* Mode = World ? Cast<AFortGameModeAthena>(World->GetAuthGameMode()) : nullptr;
		const AFortGameStateAthena* State = World ? World->GetGameState<AFortGameStateAthena>() : nullptr;
		const UFortPlaylistAthena* Playlist = State ? State->GetCurrentPlaylistAthena() : nullptr;
		if (!Mode || RosterMode.Get() != Mode || !Playlist || Playlist->MaxTeamSize <= 1 ||
			Playlist->DBNOType == EDBNOType::Off ||
			(Playlist->DBNOType == EDBNOType::NotWhenRespawning && Playlist->RespawnType != EAthenaRespawnType::None) ||
			Player->bIsDBNO || Player->bIsDying || !Player->HasAuthority())
		{
			return false;
		}

		if (!HasStandingTeammate(Player))
		{
			return false;
		}

		ReleaseCarryConnections(Player);
		Player->bWasCrouchedBeforeDBNO = Player->bIsCrouched;
		Player->bIsDBNO = true;
		Player->StopJumping();
		Player->GetCharacterMovement()->bWantsToCrouch = true;
		const float KnockTime = World->GetTimeSeconds();
		Player->DBNOStartTime = KnockTime;
		Player->DBNORevivalStacking = 0;
		Player->SetHealth(100.0f);
		Player->OnPlayerStartDBNO.Broadcast();
		Player->OnDBNOStateChanged.Broadcast(Player, true);
		Player->ForceNetUpdate();

		const TSharedRef<FTimerHandle> BleedTimer = MakeShared<FTimerHandle>();
		const TWeakObjectPtr<AController> WeakInstigator(InstigatedBy);
		const TWeakObjectPtr<AActor> WeakCauser(DamageCauser);
		const TWeakObjectPtr<AFortPlayerPawnAthena> WeakPlayer(Player);
		const TWeakObjectPtr<UWorld> WeakWorld(World);
		World->GetTimerManager().SetTimer(*BleedTimer, FTimerDelegate::CreateLambda(
			[BleedTimer, WeakWorld, WeakPlayer, WeakInstigator, WeakCauser, DamageTags, KnockTime]()
			{
				AFortPlayerPawnAthena* Downed = WeakPlayer.Get();
				if (!Downed || !Downed->bIsDBNO || Downed->bIsDying || Downed->DBNOStartTime != KnockTime)
				{
					if (UWorld* CurrentWorld = WeakWorld.Get())
					{
						CurrentWorld->GetTimerManager().ClearTimer(*BleedTimer);
					}
					return;
				}
				if (Downed->bIsBeingDBNOCarried &&
					(!IsValid(Downed->DBNOHoisterData.DBNOHoister) || !Downed->DBNOHoisterData.DBNOHoister->GetController()))
				{
					ReleaseCarryConnections(Downed);
				}
				const float Damage = HasStandingTeammate(Downed) ? 2.0f : Downed->GetHealth();
				FortAthenaHealth::ApplyDamage(Downed, Damage, DamageTags, WeakInstigator.Get(), WeakCauser.Get(), true);
			}), 1.2f, true);
		return true;
	}
}

namespace
{
	void ExecClearReviveInteraction(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_OBJECT(AActor, InteractingItem);
		P_GET_UBOOL(bClearInteraction);
		P_FINISH;
		AFortPlayerPawnAthena* Helper = Cast<AFortPlayerPawnAthena>(Context);
		if (bClearInteraction && Helper && Helper->HasAuthority() &&
			(!InteractingItem || InteractingItem == Helper->ItemInteractionActor))
		{
			Helper->ItemInteractionActor = nullptr;
			Helper->ForceNetUpdate();
		}
	}

	bool CanRevive(AController* Reviver, AFortPlayerPawnAthena* Target)
	{
		const AFortPawn* Helper = Reviver ? Cast<AFortPawn>(Reviver->GetPawn()) : nullptr;
		if (!IsValid(Target) || !IsValid(Helper) || Helper == Target || !Target->HasAuthority() ||
			!Target->bIsDBNO || Target->bIsDying || Target->bIsBeingDBNOCarried || !Target->GetController() ||
			Helper->bIsDBNO || Helper->bIsDying || Helper->GetHealth() <= 0.0f ||
			Helper->GetWorld() != Target->GetWorld())
		{
			return false;
		}

		const AFortPlayerStateAthena* HelperState = Cast<AFortPlayerStateAthena>(Helper->GetPlayerState());
		const AFortPlayerStateAthena* TargetState = Cast<AFortPlayerStateAthena>(Target->GetPlayerState());
		constexpr float ReviveRange = 250.0f;
		return HelperState && TargetState && HelperState->TeamIndex == TargetState->TeamIndex &&
			FVector::DistSquared(Helper->GetActorLocation(), Target->GetActorLocation()) <= FMath::Square(ReviveRange) &&
			Reviver->LineOfSightTo(Target);
	}
}

bool FortAthenaHealth::TryStartRevive(AController* Reviver, AFortPawn* Target)
{
	AFortPlayerPawnAthena* Downed = Cast<AFortPlayerPawnAthena>(Target);
	AFortPlayerPawnAthena* HelperPawn = Reviver ? Cast<AFortPlayerPawnAthena>(Reviver->GetPawn()) : nullptr;
	if (!IsEnabled() || !HelperPawn || HelperPawn->bIsDBNOCarrying || HelperPawn->ItemInteractionActor ||
		!CanRevive(Reviver, Downed) || Downed->DBNORevivalStacking != 0)
	{
		return false;
	}

	UWorld* World = Downed->GetWorld();
	const float ReviveDuration = Downed->ReviveFromDBNOTime > 0.0f ? Downed->ReviveFromDBNOTime : 10.0f;
	const float EndTime = World->GetTimeSeconds() + ReviveDuration;
	const float KnockTime = Downed->DBNOStartTime;
	HelperPawn->ItemInteractionActor = Downed;
	HelperPawn->ForceNetUpdate();
	Downed->DBNORevivalStacking = 1;
	Downed->ForceNetUpdate();

	const TSharedRef<FTimerHandle> Timer = MakeShared<FTimerHandle>();
	const TWeakObjectPtr<AFortPlayerPawnAthena> WeakTarget(Downed);
	const TWeakObjectPtr<AController> WeakReviver(Reviver);
	const TWeakObjectPtr<AFortPlayerPawnAthena> WeakHelper(HelperPawn);
	const TWeakObjectPtr<UWorld> WeakWorld(World);
	World->GetTimerManager().SetTimer(*Timer, FTimerDelegate::CreateLambda(
		[Timer, WeakWorld, WeakTarget, WeakReviver, WeakHelper, EndTime, KnockTime]()
		{
			UWorld* CurrentWorld = WeakWorld.Get();
			if (!CurrentWorld)
			{
				return;
			}

			AFortPlayerPawnAthena* Player = WeakTarget.Get();
			AController* Helper = WeakReviver.Get();
			AFortPlayerPawnAthena* CurrentHelperPawn = WeakHelper.Get();
			const bool bSameKnock = Player && Player->DBNOStartTime == KnockTime;
			const bool bCanContinue = bSameKnock && Helper && CurrentHelperPawn &&
				Helper->GetPawn() == CurrentHelperPawn && CurrentHelperPawn->ItemInteractionActor == Player &&
				!CurrentHelperPawn->bIsDBNOCarrying && CanRevive(Helper, Player);
			const bool bFinished = CurrentWorld->GetTimeSeconds() >= EndTime;
			if (bCanContinue && !bFinished)
			{
				return;
			}

			const bool bOwnsInteraction = CurrentHelperPawn && CurrentHelperPawn->ItemInteractionActor == WeakTarget.Get(true);
			CurrentWorld->GetTimerManager().ClearTimer(*Timer);
			if (bOwnsInteraction)
			{
				CurrentHelperPawn->ItemInteractionActor = nullptr;
				CurrentHelperPawn->ForceNetUpdate();
			}
			if (bSameKnock)
			{
				Player->DBNORevivalStacking = 0;
				if (bCanContinue)
				{
					Player->SetHealth(30.0f);
					Player->bIsDBNO = false;
					Player->bPlayedDying = false;
					if (!Player->bWasCrouchedBeforeDBNO)
					{
						Player->UnCrouch();
					}
					Player->OnDBNOStateChanged.Broadcast(Player, false);
				}
				Player->ForceNetUpdate();
			}
		}), 0.1f, true);
	return true;
}

bool FortAthenaHealth::GetSingleAliveTeam(AFortGameModeAthena* GameMode, TArray<AFortPlayerControllerAthena*>& OutTeam)
{
	OutTeam.Reset();
	if (!GameMode || RosterMode.Get() != GameMode) return false;
	int32 Team = INDEX_NONE;
	bool bHasStandingPlayer = false;
	for (const TWeakObjectPtr<AFortPlayerControllerAthena>& Weak : GetAlivePlayers())
	{
		AFortPlayerControllerAthena* PC = Weak.Get();
		AFortPlayerStateAthena* PS = PC ? Cast<AFortPlayerStateAthena>(PC->PlayerState) : nullptr;
		if (!PS) continue;
		if (Team == INDEX_NONE) Team = PS->TeamIndex;
		else if (Team != PS->TeamIndex) { OutTeam.Reset(); return false; }
		OutTeam.Add(PC);
		bHasStandingPlayer |= IsStanding(PC);
	}
	if (!bHasStandingPlayer) OutTeam.Reset();
	return bHasStandingPlayer;
}

int32 FortAthenaHealth::GetAliveTeamCount(AFortGameModeAthena* GameMode)
{
	TSet<int32> Teams;
	if (GameMode && RosterMode.Get() == GameMode)
	{
		for (const TWeakObjectPtr<AFortPlayerControllerAthena>& Weak : GetAlivePlayers())
		{
			const AFortPlayerControllerAthena* PC = Weak.Get();
			if (const AFortPlayerStateAthena* PS = PC ? Cast<AFortPlayerStateAthena>(PC->PlayerState) : nullptr)
				Teams.Add(PS->TeamIndex);
		}
	}
	return Teams.Num();
}

void FortAthenaHealth::FinalizeSurvival(AFortGameModeAthena* GameMode, AFortPlayerStateAthena* PS)
{
	AFortGameStateAthena* GS = GameMode ? GameMode->GetGameState<AFortGameStateAthena>() : nullptr;
	if (!PS || !GS || RosterMode.Get() != GameMode || ParticipationStart < 0.0f) return;
	PS->SecondsAlive = FMath::Max(0, FMath::FloorToInt(GS->GetServerWorldTimeSeconds() - ParticipationStart));
	PS->NumMinutesAlive = PS->SecondsAlive / 60;
	PS->OnRep_NumMinutesAlive();
}

void FortAthenaHealth::SendPartialTeamStats(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Controller, int32 Place, bool bPadTotal)
{
	if (!GameMode || RosterMode.Get() != GameMode || !Controller || !bRosterFrozen) return;
	// Detailed stats and rewards require recovered semantics.
	FAthenaMatchTeamStats TeamStats{};
	TeamStats.Place = Place;
	TeamStats.TotalPlayers = bPadTotal ? FMath::Max(MatchParticipants, Place) : MatchParticipants;
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

	void InitializeAthenaHeroAttributes(AFortPlayerStateAthena& PlayerState,
		AFortPlayerControllerAthena& Controller, AFortGameModeAthena& Mode,
		UFortAbilitySystemComponent& ASC)
	{
		// ASC survives replacement pawns. Do not refill health/stamina on repeated
		// SetupPlayer calls (including ServerResetAbilitySystemComponent).
		static TSet<TWeakObjectPtr<UFortAbilitySystemComponent>> InitializedASCs;
		for (auto It = InitializedASCs.CreateIterator(); It; ++It)
			if (!It->IsValid()) { It.RemoveCurrent(); }
		if (InitializedASCs.Contains(&ASC)) { return; }
		UFortHeroType* HeroType = PlayerState.HeroType;
		AFortGameStateAthena* State = Mode.GetGameState<AFortGameStateAthena>();
		if (!HeroType || !State)
		{
			UE_LOG(LogFortHealth, Warning, TEXT("Attributes: awaiting selected HeroType/GameState for %s"), *PlayerState.GetName());
			return;
		}
		// The actual item factory uses the inherited level and native clamp defaults.
		// Keep it rooted across synchronous curve/table loads even without registered info.
		TStrongObjectPtr<UFortHero> Hero(HeroType->NativeCreateTemporaryHero142017d90(1));
		if (!Hero.IsValid()) { return; }
		if (Controller.MyPlayerInfo)
			Controller.MyPlayerInfo->TempAthenaMenuHeroInstance = Hero.Get();

		const FFortAttributeInitializationKey& Key = HeroType->GetAttributeInitKey();
		TSet<FProperty*> InitializedProperties;
		const FFortPlayerPawnStats* Stats = nullptr;
		{
			// Athena-specific server recreation of the original homebase attribute
			// phase only. Campaign/team effects and zone clamp effects remain separate.
			FScopedAggregatorOnDirtyBatch Batch;
			if (const UFortCharacterAttrSet* CharacterSet = ASC.GetSet<UFortCharacterAttrSet>())
			{
				static FProperty* LevelProperty = FindFieldChecked<FProperty>(
					UFortCharacterAttrSet::StaticClass(), TEXT("MaxCharacterLevel"));
				FGameplayEffectAttributeCaptureSpec Capture(FGameplayEffectAttributeCaptureDefinition(
					FGameplayAttribute(LevelProperty), EGameplayEffectAttributeCaptureSource::Target, false));
				ASC.CaptureAttributeForGameplayEffect(Capture);
				FAggregatorEvaluateParameters Evaluation;
				float EvaluatedLevel = 0.0f;
				Capture.AttemptCalculateAttributeMagnitude(Evaluation, EvaluatedLevel);
				const FFortGameplayAttributeData& Attribute = CharacterSet->MaxCharacterLevel;
				if (Attribute.Minimum <= EvaluatedLevel)
				{
					if (Attribute.Maximum <= EvaluatedLevel) { EvaluatedLevel = Attribute.Maximum; }
				}
				else { EvaluatedLevel = Attribute.Minimum; }
				Hero->NativeRecoveredInt32_0x228 = static_cast<int32>(EvaluatedLevel);
			}
			if (!Key.AttributeInitCategory.IsNone())
			{
				const FAttributeSetInitter* Initter = UAbilitySystemGlobals::Get().GetAttributeSetInitter();
				if (Initter)
					static_cast<const FFortAttributeSetInitter*>(Initter)->InitAttributeSetDefaults(
						&ASC, Key, static_cast<float>(Hero->GetLevel()), false, true, InitializedProperties);
			}
			else
			{
				UE_LOG(LogFortHealth, Warning, TEXT("Attributes: no initialization category for %s"), *HeroType->GetName());
			}
			Stats = State->GetPlaylistHeroStats(HeroType->LegacyStatHandle.RowName);
			if (!Stats) { Stats = HeroType->GetHeroLegacyStats(); }
			if (Stats) { FortPlayerStatFallback::ApplyForPlayerState(PlayerState, *Stats, InitializedProperties); }
		}
		if (Stats || InitializedProperties.Num() != 0) { InitializedASCs.Add(&ASC); }
		else
		{
			UE_LOG(LogFortHealth, Warning, TEXT("Attributes: no hero curves or legacy stats resolved for %s"), *HeroType->GetName());
		}
		const UFortMovementSet& Movement = *PlayerState.AttributeSets.MovementSet;
		UE_LOG(LogFortHealth, Display,
			TEXT("Attributes: player=%s hero=%s key=%s/%s level=%d clamp=%d ASC=%s initialized=%d legacy=%d walk=%.2f run=%.2f sprint=%.2f backward=%.3f multiplier=%.3f"),
			*PlayerState.GetName(), *HeroType->GetName(), *Key.AttributeInitCategory.ToString(),
			*Key.AttributeInitSubCategory.ToString(), Hero->GetLevel(), Hero->NativeRecoveredInt32_0x228,
			*ASC.GetClass()->GetName(), InitializedProperties.Num(), Stats != nullptr,
			Movement.WalkSpeed.GetCurrentValue(), Movement.RunSpeed.GetCurrentValue(),
			Movement.SprintSpeed.GetCurrentValue(), Movement.BackwardSpeedMultiplier.GetCurrentValue(),
			Movement.SpeedMultiplier.GetCurrentValue());
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
		ASC = NewObject<UFortAbilitySystemComponentAthena>(PlayerState, TEXT("AbilitySystemComponent"), RF_DefaultSubObject);  // see AddNamedSet
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

		UServerActorDamageListener::Bind(*Pawn);
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
		Pawn->ControlResistanceSet = ControlSet;
	}

	UFortMovementSet* MovementSet = AddNamedSet<UFortMovementSet>(ASC, PlayerState, TEXT("MovementSet"));
	if (MovementSet)
	{
		PlayerState->AttributeSets.MovementSet = MovementSet;
		Pawn->MovementSet = MovementSet;
	}

	UFortAdvancedMovementSet* AdvMovementSet = AddNamedSet<UFortAdvancedMovementSet>(ASC, PlayerState, TEXT("AdvancedMovementSet"));
	if (AdvMovementSet)
	{
		PlayerState->AttributeSets.AdvancedMovementSet = AdvMovementSet;
		Pawn->AdvancedMovementSet = AdvMovementSet;
	}

	// These are part of the original PlayerState attribute family. Character level
	// must use its native constructor value, not a synthetic Athena level.
	PlayerState->AttributeSets.CharacterAttrSet = AddNamedSet<UFortCharacterAttrSet>(ASC, PlayerState, TEXT("CharacterAttrSet"));
	PlayerState->AttributeSets.PlayerAttrSet = AddNamedSet<UFortPlayerAttrSet>(ASC, PlayerState, TEXT("PlayerAttrSet"));

	InitializeAthenaHeroAttributes(*PlayerState, *Controller, *GameMode, *ASC);

	// GUESS (backpack-full): the client's pickup fit check adds WorldInventorySizeBonus from this set to inventory
	// capacity. Without the set the capacity may evaluate to 0 and every gun is refused with "Backpack full".
	if (UFortHomebaseSet* Homebase = AddNamedSet<UFortHomebaseSet>(ASC, PlayerState, TEXT("HomebaseSet")))
	{
		Homebase->WorldInventorySizeBonus.SetBaseValue(50.0f);
		Homebase->WorldInventorySizeBonus.SetCurrentValue(50.0f);
		Homebase->InventorySizeBonus.SetBaseValue(50.0f);
		Homebase->InventorySizeBonus.SetCurrentValue(50.0f);
		UE_LOG(LogFortHealth, Display, TEXT("Attributes: HomebaseSet inventory bonus set for %s"), *PlayerState->GetName());
	}

	// GUESS 2 (backpack-full): strip pickup-blocking tags. The client refuses a pickup when its pawn ASC owns
	// Gameplay.Status.DisablePickup.<Type> or Gameplay.Action.Player.BlockPickup. Log what is found.
	if (UWorld* TagWorld = Controller->GetWorld())
	{
		TWeakObjectPtr<UFortAbilitySystemComponent> WeakASC(ASC);
		FTimerHandle TagHandle;
		TagWorld->GetTimerManager().SetTimer(TagHandle, FTimerDelegate::CreateLambda([WeakASC]()
		{
			UFortAbilitySystemComponent* A = WeakASC.Get();
			if (!A) { return; }
			FGameplayTagContainer Owned;
			A->GetOwnedGameplayTags(Owned);
			FGameplayTagContainer Blocking;
			for (const FGameplayTag& Tag : Owned)
			{
				const FString Name = Tag.ToString();
				if (Name.Contains(TEXT("DisablePickup")) || Name.Contains(TEXT("BlockPickup")))
				{
					Blocking.AddTag(Tag);
				}
			}
			if (Blocking.Num() == 0) { return; }
			UE_LOG(LogFortHealth, Display, TEXT("PickupTags: stripping %s"), *Blocking.ToStringSimple());
			A->RemoveActiveEffectsWithGrantedTags(Blocking);
			for (const FGameplayTag& Tag : Blocking)
			{
				A->SetLooseGameplayTagCount(Tag, 0);
			}
		}), 1.0f, true);
	}

	// Retain the existing server health bootstrap only when neither hero curves nor
	// legacy stats supplied usable health. Successful original initialization owns it.
	if (HealthSet)
	{
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

	// Publish the same values that authoritative movement now reads from GAS.
	if (AFortPlayerPawnAthena* AthenaPawn = Cast<AFortPlayerPawnAthena>(Pawn))
	{
		AthenaPawn->AttributeReplicationProxy.WalkSpeed = MovementSet->WalkSpeed.GetCurrentValue();
		AthenaPawn->AttributeReplicationProxy.RunSpeed = MovementSet->RunSpeed.GetCurrentValue();
		AthenaPawn->AttributeReplicationProxy.SprintSpeed = MovementSet->SprintSpeed.GetCurrentValue();
		AthenaPawn->AttributeReplicationProxy.FlySpeed = MovementSet->FlySpeed.GetCurrentValue();
		AthenaPawn->AttributeReplicationProxy.CrouchedRunSpeed = MovementSet->CrouchedRunSpeed.GetCurrentValue();
		AthenaPawn->AttributeReplicationProxy.CrouchedSprintSpeed = MovementSet->CrouchedSprintSpeed.GetCurrentValue();
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
			UServerDamageEffect::BindDamageAttribute(*ASC);
			UServerRecoveryEffect::BindHealingAttribute(*ASC);
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
					const float LethalDamage = FMath::Max(0.0f, Data.OldValue - Data.NewValue);
					if (Data.GEModData)
					{
						const FGameplayEffectSpec& Spec = Data.GEModData->EffectSpec;
						Spec.GetAllAssetTags(DeathTags);
						DeathTags.AppendTags(*Spec.CapturedSourceTags.GetAggregatedTags());
						if (Spec.GetContext().IsValid())
						{
							AActor* Source = Spec.GetContext().GetOriginalInstigator();
							AController* SourceController = Cast<AController>(Source);
							if (APawn* SourcePawn = Cast<APawn>(Source))
							{
								SourceController = SourcePawn->GetController();
							}
							else if (APlayerState* SourceState = Cast<APlayerState>(Source))
							{
								SourceController = Cast<AController>(SourceState->GetOwner());
							}
							WeakInstigator = SourceController;
							WeakCauser = Spec.GetContext().GetEffectCauser();
						}
					}
					TWeakObjectPtr<AFortPawn> WeakPawn(PawnAtChange);
					const bool bWasDowned = PawnAtChange->bIsDBNO;
					// The health delegate can fire inside GAS before the HealthSet's
					// synchronous DBNO interception. Recheck after that stack returns.
					World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda(
						[WeakASC, WeakPawn, WeakInstigator, WeakCauser, ChangeSerial, Serial, DeathTags, LethalDamage, bWasDowned]()
						{
							UAbilitySystemComponent* CurrentASC = WeakASC.Get();
							AFortPawn* CurrentPawn = WeakPawn.Get();
							if (*ChangeSerial != Serial || !CurrentASC || !CurrentPawn ||
								CurrentASC->GetAvatarActor() != CurrentPawn || CurrentPawn->bIsDying ||
								(!bWasDowned && CurrentPawn->bIsDBNO) || CurrentPawn->GetHealth() > 0.0f) { return; }
							if (!TryKnockPlayer(CurrentPawn, DeathTags, WeakInstigator.Get(), WeakCauser.Get()))
								{
									FortAthenaHealth::HandlePlayerEliminated(CurrentPawn, DeathTags, WeakInstigator.Get(), WeakCauser.Get(), LethalDamage);
								}
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
	RegisterAlivePlayer(GameMode, Controller);
}

void FortAthenaHealth::RegisterAlivePlayer(AFortGameModeAthena* GameMode, AFortPlayerControllerAthena* Controller)
{
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
	if (!IsEnabled() || !VictimPawn || !VictimPawn->HasAuthority() || VictimPawn->bIsDying ||
		!FMath::IsFinite(DamageAmount) || DamageAmount <= 0.0f)
	{
		return;
	}

	UAbilitySystemComponent* ASC = VictimPawn->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	// Non-player pawns can own an ASC with a configured health set before
	// their pawn-side health pointer has been synchronized.
	if (!VictimPawn->bHealthSynced && !Cast<AFortPlayerPawn>(VictimPawn) && ASC->GetOwner() == VictimPawn)
	{
		if (const UFortHealthSet* HealthSet = ASC->GetSet<UFortHealthSet>())
		{
			if (!ASC->GetAvatarActor())
			{
				ASC->InitAbilityActorInfo(VictimPawn, VictimPawn);
			}
			if (ASC->GetAvatarActor() == VictimPawn)
			{
				VictimPawn->HealthSet = const_cast<UFortHealthSet*>(HealthSet);
				VictimPawn->bHealthSynced = true;
			}
		}
	}

	static const FGameplayAttribute Health(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Health")));
	if (ASC->GetAvatarActor() != VictimPawn || !ASC->HasAttributeSetForAttribute(Health) || !VictimPawn->bHealthSynced)
	{
		return;
	}

	const float CurrentShield = VictimPawn->GetShield();
	const float OldHealth = VictimPawn->GetHealth();
	const bool bWasDowned = VictimPawn->bIsDBNO;

	// GAS captures source tags and attributes from the actor in the effect context.
	// Keep controller attribution separately for elimination and match statistics.
	AActor* SourceActor = InstigatedBy;
	APawn* SourcePawn = InstigatedBy ? InstigatedBy->GetPawn() : nullptr;
	if (!SourcePawn && DamageCauser)
	{
		SourcePawn = DamageCauser->GetInstigator();
	}
	if (UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(SourcePawn))
	{
		SourceActor = SourcePawn;
	}
	else if (!SourceActor && UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(DamageCauser))
	{
		SourceActor = DamageCauser;
	}
	FGameplayEffectContextHandle Context(UAbilitySystemGlobals::Get().AllocGameplayEffectContext());
	Context.AddInstigator(SourceActor, DamageCauser);
	FGameplayEffectSpec Spec(GetDefault<UServerDamageEffect>(), Context, 1.0f);
	Spec.DynamicAssetTags.AppendTags(DamageTags);
	Spec.SetSetByCallerMagnitude(FName(TEXT("Damage")), DamageAmount);
	Spec.SetSetByCallerMagnitude(FName(TEXT("IgnoreShield")), bIgnoreShield ? 1.0f : 0.0f);
	ASC->ApplyGameplayEffectSpecToSelf(Spec);

	const float NewHealth = VictimPawn->GetHealth();
	const float ShieldDamage = CurrentShield - VictimPawn->GetShield();
	const float HealthDamage = OldHealth - NewHealth;
	if (ShieldDamage <= 0.0f && HealthDamage <= 0.0f)
	{
		return;
	}

	// Multicast damage cues to client (hit numbers, hit markers, shield shatter).
	FAthenaBatchedDamageGameplayCues_Shared SharedCues{};
	SharedCues.Location = FVector_NetQuantize10(VictimPawn->GetActorLocation());
	SharedCues.Normal = FVector_NetQuantizeNormal(FVector::UpVector);
	SharedCues.Magnitude = DamageAmount;
	SharedCues.bIsShield = (ShieldDamage > 0.0f);
	SharedCues.bIsShieldDestroyed = (ShieldDamage > 0.0f && VictimPawn->GetShield() <= 0.0f);
	const bool bFatal = NewHealth <= 0.0f && (bWasDowned || !VictimPawn->bIsDBNO) &&
		!TryKnockPlayer(VictimPawn, DamageTags, InstigatedBy, DamageCauser);
	SharedCues.bIsFatal = bFatal;
	SharedCues.bIsValid = true;

	FAthenaBatchedDamageGameplayCues_NonShared NonSharedCues{};
	NonSharedCues.HitActor = VictimPawn;

	// Environmental damage (storm, falling) has no instigator or causer. The original applies it through a gameplay effect
	// without the weapon hit cue; sending the batched hit cue here played a gunshot-like sound on the client.
	// Fall damage is the exception: without any cue the victim gets no hit feedback at all (health bar only), so it
	// sends the batched cue (hit number / damage reaction) like other damage. Storm damage stays silent.
	const FGameplayTag FallDamageTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x358);
	const bool bFallDamage = FallDamageTag.IsValid() && DamageTags.HasTag(FallDamageTag);
	if (InstigatedBy || DamageCauser || bFallDamage)
	{
		VictimPawn->NetMulticast_Athena_BatchedDamageCues(SharedCues, NonSharedCues);
	}

	UE_LOG(LogFortHealth, Display, TEXT("Health: Damage %.1f applied to %s (Shield: -%.1f, Health: -%.1f) -> Health: %.1f, Shield: %.1f"),
		DamageAmount, *VictimPawn->GetName(), ShieldDamage, HealthDamage, NewHealth, VictimPawn->GetShield());

	if (bFatal && !VictimPawn->bIsDying)
	{
		HandlePlayerEliminated(VictimPawn, DamageTags, InstigatedBy, DamageCauser, DamageAmount);
	}
}

namespace
{
	void NotifyPawnDeath(AFortPawn* VictimPawn, AController* InstigatedBy, AActor* DamageCauser, float LethalDamage)
	{
		ReleaseCarryConnections(VictimPawn);
		VictimPawn->bWasDBNOOnDeath = VictimPawn->bIsDBNO;
		if (UCharacterMovementComponent* MoveComp = VictimPawn->GetCharacterMovement())
		{
			MoveComp->DisableMovement();
		}
		VictimPawn->SetCanBeDamaged(false);
		VictimPawn->OnDied.Broadcast(VictimPawn, LethalDamage, InstigatedBy, DamageCauser, VictimPawn->GetActorLocation(), nullptr, NAME_None, FVector::ZeroVector);
	}
}

void FortAthenaHealth::HandlePlayerEliminated(AFortPawn* VictimPawn, const FGameplayTagContainer& DamageTags,
                                             AController* InstigatedBy, AActor* DamageCauser, float LethalDamage)
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
	// AI and other non-player pawns have no match placement to finalize.
	if (!VictimPC && !VictimState && !Cast<APlayerController>(VictimPawn->GetController()))
	{
		VictimPawn->bIsDying = true;
		NotifyPawnDeath(VictimPawn, InstigatedBy, DamageCauser, LethalDamage);
		return;
	}
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
	NotifyPawnDeath(VictimPawn, InstigatedBy, DamageCauser, LethalDamage);

	AFortPlayerStateAthena* KillerState = InstigatedBy ? Cast<AFortPlayerStateAthena>(InstigatedBy->PlayerState) : nullptr;
	AFortPawn* KillerPawn = InstigatedBy ? Cast<AFortPawn>(InstigatedBy->GetPawn()) : nullptr;
	AFortWeapon* FinishingWeapon = Cast<AFortWeapon>(DamageCauser);
	if (!FinishingWeapon && KillerPawn)
	{
		FinishingWeapon = KillerPawn->CurrentWeapon;
	}
	UFortWeaponItemDefinition* KillerWeapon = FinishingWeapon ? FinishingWeapon->WeaponData : nullptr;

	// 1. Build FFortPlayerDeathReport (D6 §2 step 1).
	FFortPlayerDeathReport Report{};
	Report.ServerTimeForRespawn = -1.0f;
	Report.ServerTimeForResurrect = -1.0f;
	Report.LethalDamage = LethalDamage;
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

	// Drop the victim's inventory where they died (Report.bDroppedBackpack says the client may show the backpack drop).
	FortAthenaLootTransfer::DropAllOnDeath(VictimPC, VictimPawn);

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
	// stand-in: a sole player who dies has nobody to place behind, which made the client treat the death as place #1 and show
	// Victory Royale straight after a fatal fall (user report 2026.11.15). With no one left the death is reported as 2nd of at
	// least 2 so the elimination screen is used, and the stats (which switch the client to the end screen) are sent after a
	// short delay so the death presentation (health drop, death cue, death camera) is visible first.
	const bool bNobodyLeft = NewPlayersLeft == 0;
	// Team playlists place by teams left, not players left (stand-in: a victim whose teammates are alive is provisionally placed the same).
	const UFortPlaylistAthena* VictimPlaylist = GameState ? GameState->GetCurrentPlaylistAthena() : nullptr;
	const bool bTeamPlaylist = VictimPlaylist && VictimPlaylist->MaxTeamSize > 1;
	const int32 Place = bNobodyLeft ? 2 : (bTeamPlaylist ? GetAliveTeamCount(GameMode) : NewPlayersLeft) + 1;
	if (VictimState)
	{
		VictimState->Place = Place;
		VictimState->OnRep_Place();
		FinalizeSurvival(GameMode, VictimState);
	}

	static const float StatsDelay = []() { float V = 3.5f; FParse::Value(FCommandLine::Get(), TEXT("FortniteDeathStatsDelay="), V); return FMath::Max(0.0f, V); }();
	if (StatsDelay > 0.0f && World && VictimPC)
	{
		TWeakObjectPtr<AFortGameModeAthena> WeakMode(GameMode);
		TWeakObjectPtr<AFortPlayerControllerAthena> WeakPC(VictimPC);
		FTimerHandle Handle;
		World->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([WeakMode, WeakPC, Place, bNobodyLeft]()
		{
			if (WeakMode.IsValid() && WeakPC.IsValid())
				SendPartialTeamStats(WeakMode.Get(), WeakPC.Get(), Place, bNobodyLeft);
		}), StatsDelay, false);
	}
	else
	{
		SendPartialTeamStats(GameMode, VictimPC, Place, bNobodyLeft);
	}

	// Spectate after death. stand-in: Athena allows spectating after death (constructor evidence in G9 section 2) but the original target
	// selection and ServerSpectatePlayer bodies are not recovered; after a short delay the victim's view follows the killer if alive,
	// otherwise the first living player. Untested with a client; the death camera is kept when nobody is left.
	if (World && VictimPC && NewPlayersLeft > 0)
	{
		TWeakObjectPtr<AFortPlayerControllerAthena> WeakVictim(VictimPC);
		TWeakObjectPtr<AFortPawn> WeakKiller(KillerPawn);
		FTimerHandle SpectateHandle;
		World->GetTimerManager().SetTimer(SpectateHandle, FTimerDelegate::CreateLambda([WeakVictim, WeakKiller]()
		{
			AFortPlayerControllerAthena* Victim = WeakVictim.Get();
			if (!Victim)
				return;
			AActor* Target = nullptr;
			if (AFortPawn* Killer = WeakKiller.Get())
			{
				if (!Killer->bIsDying && Killer != Victim->GetPawn())
					Target = Killer;
			}
			if (!Target)
			{
				for (const TWeakObjectPtr<AFortPlayerControllerAthena>& Other : GetAlivePlayers())
				{
					AFortPlayerControllerAthena* OtherPC = Other.Get();
					AFortPawn* OtherPawn = OtherPC ? Cast<AFortPawn>(OtherPC->GetPawn()) : nullptr;
					if (OtherPawn && !OtherPawn->bIsDying)
					{
						Target = OtherPawn;
						break;
					}
				}
			}
			if (Target)
			{
				Victim->SetViewTargetWithBlend(Target, 0.5f);
				UE_LOG(LogFortHealth, Display, TEXT("Health: %s spectating %s after death"), *Victim->GetName(), *Target->GetName());
			}
		}), 5.0f, false);
	}

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

	if (UFunction* Interaction = AFortPlayerPawnAthena::StaticClass()->FindFunctionByName(TEXT("ServerSetInteractingItem")))
	{
		Interaction->SetNativeFunc(&ExecClearReviveInteraction);
	}

	if (UFunction* Hoist = AFortPlayerPawn::StaticClass()->FindFunctionByName(TEXT("ServerHoistDBNOPlayer")))
	{
		Hoist->SetNativeFunc(&ExecHoistDownedPlayer);
	}
	if (UFunction* Drop = AFortPlayerPawn::StaticClass()->FindFunctionByName(TEXT("ServerDropCarriedPlayer")))
	{
		Drop->SetNativeFunc(&ExecDropCarriedPlayer);
	}
	if (UFunction* Throw = AFortPlayerPawn::StaticClass()->FindFunctionByName(TEXT("ServerThrowCarriedPlayer")))
	{
		Throw->SetNativeFunc(&ExecThrowCarriedPlayer);
	}

	bInstalled = true;
}

// Global-namespace wrappers for behaviour snippets (.inc bodies).
void FortAthenaHealth_ReleaseCarryOnUnPossess(AFortPawn* Pawn)
{
	if (Pawn->HasAuthority() && FortAthenaHealth::IsEnabled())
	{
		ReleaseCarryConnections(Pawn);
	}
}

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
