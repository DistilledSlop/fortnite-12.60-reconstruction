#if WITH_DEV_AUTOMATION_TESTS

#include "ABuildingWall.h"
#include "AIController.h"
#include "AFortPawn.h"
#include "AFortAIPawn.h"
#include "AFortGameModeAthena.h"
#include "AFortGameStateAthena.h"
#include "AFortPlayerControllerAthena.h"
#include "AFortPlayerPawnAthena.h"
#include "AFortPlayerStateAthena.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FortAthenaHealth.h"
#include "FortAthenaLootSearch.h"
#include "FortHarvestBuilding.h"
#include "FortNativeGameplayTagsRuntime.h"
#include "GameFramework/DamageType.h"
#include "GameplayEffectExtension.h"
#include "Misc/AutomationTest.h"
#include "ServerDamageExecution.h"
#include "UFortAbilitySystemComponent.h"
#include "UFortControllerComponent_Interaction.h"
#include "UFortDamageFormulaExecutionCalculation.h"
#include "UFortDamageSet.h"
#include "UFortHealthSet.h"
#include "UFortGameplayAbility_RangedWeapon.h"
#include "UFortPlaylistAthena.h"
#include "UFortMovementComp_CharacterAthena.h"

// UE 4.25 discovers dedicated-server tests under ClientContext.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FServerDamageExecutionTest, "Fortnite.Server.Damage.UnrealEvent",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

bool FServerDamageExecutionTest::RunTest(const FString&)
{
	if (!IsRunningDedicatedServer() || !FortAthenaHealth::IsEnabled())
	{
		AddInfo(TEXT("Requires a dedicated server launched with -FortniteOriginalClient."));
		return true;
	}

	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World() && Context.World()->GetNetMode() == NM_DedicatedServer)
		{
			World = Context.World();
			break;
		}
	}
	if (!TestNotNull(TEXT("Dedicated server world"), World))
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFortPlayerPawnAthena* Crawler = World->SpawnActor<AFortPlayerPawnAthena>(
		FVector(0, 0, 11000), FRotator::ZeroRotator, SpawnParameters);
	if (TestNotNull(TEXT("Downed movement test pawn"), Crawler))
	{
		UFortMovementComp_CharacterAthena* Move = Cast<UFortMovementComp_CharacterAthena>(Crawler->GetCharacterMovement());
		if (TestNotNull(TEXT("Downed pawn uses Athena movement"), Move))
		{
			Move->SetMovementMode(MOVE_Walking, 0);
			Crawler->bIsDBNO = true;
			Crawler->PawnStatHandle = FDataTableRowHandle();
			TestEqual(TEXT("A downed pawn without a movement stats row can crawl"), Move->GetMaxSpeed(), 100.0f);
			Move->MinAnalogWalkSpeed = 200.0f;
			TestEqual(TEXT("Analog input cannot exceed the crawl speed"), Move->GetMinAnalogSpeed(), 100.0f);
			Move->bWantsToCrouch = false;
			Move->UpdateCharacterStateBeforeMovement(0.0f);
			TestTrue(TEXT("Downed movement keeps crouch requested"), Move->bWantsToCrouch != 0);
			Move->bWantsToCrouch = false;
			TestFalse(TEXT("A downed player cannot jump even if an incoming move clears crouch"), Move->CanAttemptJump());
		}
		Crawler->Destroy();
	}

	AFortPlayerPawnAthena* Carrier = World->SpawnActor<AFortPlayerPawnAthena>(
		FVector(0, 0, 12000), FRotator::ZeroRotator, SpawnParameters);
	AFortPlayerPawnAthena* Downed = World->SpawnActor<AFortPlayerPawnAthena>(
		FVector(175, 0, 12000), FRotator::ZeroRotator, SpawnParameters);
	AAIController* CarryController = World->SpawnActor<AAIController>();
	AAIController* DownedController = World->SpawnActor<AAIController>();
	UFortControllerComponent_Interaction* Interaction = CarryController
		? NewObject<UFortControllerComponent_Interaction>(CarryController) : nullptr;
	AFortPlayerStateAthena* CarrierState = World->SpawnActor<AFortPlayerStateAthena>();
	AFortPlayerStateAthena* DownedState = World->SpawnActor<AFortPlayerStateAthena>();
	if (TestNotNull(TEXT("Carry fixture carrier"), Carrier) && TestNotNull(TEXT("Carry fixture target"), Downed) &&
		TestNotNull(TEXT("Carry fixture interaction component"), Interaction) &&
		TestNotNull(TEXT("Downed fixture controller"), DownedController) &&
		TestNotNull(TEXT("Carry fixture carrier state"), CarrierState) && TestNotNull(TEXT("Carry fixture target state"), DownedState))
	{
		FortAthenaLootSearch::InstallRpcOverrides();
		Interaction->RegisterComponent();
		CarryController->Possess(Carrier);
		DownedController->Possess(Downed);
		Carrier->SetPlayerState(CarrierState);
		Downed->SetPlayerState(DownedState);
		CarrierState->TeamIndex = DownedState->TeamIndex = 3;
		Carrier->bAllowDBNOCarry = true;
		Interaction->ServerAttemptInteract(Downed, nullptr, TInteractionType::IT_Simple, nullptr,
			EInteractionBeingAttempted::SecondInteraction, 1);
		TestNull(TEXT("The interaction RPC rejects carrying a standing player"), Carrier->DBNOHoistee);
		Downed->bIsDBNO = true;

		Interaction->ServerAttemptInteract(Downed, nullptr, TInteractionType::IT_Simple, nullptr,
			EInteractionBeingAttempted::SecondInteraction, 2);
		if (TestTrue(TEXT("The interaction RPC picks up a nearby downed player"), Carrier->DBNOHoistee == Downed))
		{
			TestTrue(TEXT("Carry establishes both links and attaches the downed player"),
				Carrier->DBNOHoistee == Downed && Downed->DBNOHoisterData.DBNOHoister == Carrier &&
				Downed->GetAttachParentActor() == Carrier);
			FFortDBNOCarryHoisterData PreviousHoisterData{};
			Carrier->bIsDBNOCarrying = false;
			Downed->bIsBeingDBNOCarried = false;
			Carrier->OnRep_DBNOHoistee();
			Downed->OnRep_DBNOHoisterData(PreviousHoisterData);
			TestTrue(TEXT("Carry replication callbacks set both local flags"),
				Carrier->bIsDBNOCarrying && Downed->bIsBeingDBNOCarried);
			PreviousHoisterData = Downed->DBNOHoisterData;
			Carrier->DropCarriedPlayerHeightOffset = 30.0f;
			Carrier->ServerDropCarriedPlayer();
			TestEqual(TEXT("A drop uses the configured height above the carrier's capsule center"),
				Downed->GetActorLocation().Z - Carrier->GetActorLocation().Z, 30.0f);
			TestTrue(TEXT("The drop RPC removes the carry links and attachment"),
				!Carrier->DBNOHoistee && !Downed->DBNOHoisterData.DBNOHoister && !Downed->GetAttachParentActor());
			Carrier->bIsDBNOCarrying = true;
			Downed->bIsBeingDBNOCarried = true;
			Carrier->OnRep_DBNOHoistee();
			Downed->OnRep_DBNOHoisterData(PreviousHoisterData);
			TestFalse(TEXT("Release replication callbacks clear both local flags"),
				Carrier->bIsDBNOCarrying || Downed->bIsBeingDBNOCarried);
			Downed->SetActorLocation(FVector(175, 0, 12000));
			Carrier->ServerHoistDBNOPlayer(Downed);
			if (TestTrue(TEXT("The hoist RPC can pick up a dropped player"), Carrier->DBNOHoistee == Downed))
			{
				Carrier->ThrowCarriedPlayerStrengthXY = 900.0f;
				Carrier->ThrowCarriedPlayerStrengthZ = 450.0f;
				Carrier->SetActorRotation(FRotator(0.0f, 90.0f, 0.0f));
				Carrier->ServerThrowCarriedPlayer();
				TestTrue(TEXT("The throw RPC releases the player with the thrown event"),
					!Carrier->DBNOHoistee && !Downed->GetAttachParentActor() &&
					Downed->DBNOHoisterData.DBNOCarryEvent == EFortDBNOCarryEvent::Thrown);
				UCharacterMovementComponent* Movement = Downed->GetCharacterMovement();
				TestTrue(TEXT("The throw queues a character launch"), Movement->HandlePendingLaunch());
				TestTrue(TEXT("The throw applies the configured forward and upward velocities"),
					Movement->Velocity.Equals(FVector(0.0f, 900.0f, 450.0f), 0.1f));
				Movement->StopMovementImmediately();
			}
		}
	}
	const TArray<TWeakObjectPtr<AActor>> Fixtures = {
		CarryController, DownedController, Carrier, Downed, CarrierState, DownedState
	};
	if (Carrier && Downed && Interaction && DownedController && CarrierState && DownedState)
	{
		Carrier->GetCharacterMovement()->SetComponentTickEnabled(false);
		Downed->GetCharacterMovement()->SetComponentTickEnabled(false);
		Downed->SetActorLocation(FVector(175, 0, 12000));
		Downed->ReviveFromDBNOTime = 0.2f;

		UFortAbilitySystemComponent* DownedASC = NewObject<UFortAbilitySystemComponent>(Downed);
		Downed->AbilitySystemComponent = DownedASC;
		DownedASC->RegisterComponent();
		DownedASC->InitAbilityActorInfo(Downed, Downed);
		Downed->HealthSet = NewObject<UFortHealthSet>(Downed);
		DownedASC->AddAttributeSetSubobject(Downed->HealthSet);
		Downed->bHealthSynced = true;
		Downed->SetMaxHealth(100.0f);
		Downed->SetHealth(60.0f);

		Interaction->ServerAttemptInteract(Downed, nullptr, TInteractionType::IT_LongPress, nullptr,
			EInteractionBeingAttempted::FirstInteraction, 3);
		TestTrue(TEXT("The interaction RPC starts a timed revive for a standing teammate"),
			Carrier->ItemInteractionActor == Downed && Downed->DBNORevivalStacking == 1);
		Carrier->ServerSetInteractingItem(Downed, true);
		TestNull(TEXT("Clearing interaction releases the reviver's target"), Carrier->ItemInteractionActor);
		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(([this, Fixtures]()
		{
			AFortPlayerPawnAthena* Player = Cast<AFortPlayerPawnAthena>(Fixtures[3].Get());
			AController* Reviver = Cast<AController>(Fixtures[0].Get());
			if (TestNotNull(TEXT("Revive target persists after cancellation"), Player))
			{
				TestTrue(TEXT("Cancellation leaves the player downed and clears revive progress"),
					Player->bIsDBNO && Player->DBNORevivalStacking == 0);
				TestEqual(TEXT("Cancellation leaves health unchanged"), Player->GetHealth(), 60.0f);
				TestTrue(TEXT("A cancelled revive can be restarted"), FortAthenaHealth::TryStartRevive(Reviver, Player));
				Player->SetActorLocation(FVector(600, 0, 12000));
			}
		}), 0.4f));
		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(([this, Fixtures]()
		{
			AFortPlayerPawnAthena* Helper = Cast<AFortPlayerPawnAthena>(Fixtures[2].Get());
			AFortPlayerPawnAthena* Player = Cast<AFortPlayerPawnAthena>(Fixtures[3].Get());
			AController* Reviver = Cast<AController>(Fixtures[0].Get());
			if (Helper && Player && Reviver)
			{
				TestTrue(TEXT("Leaving revive range cancels the timer and clears both interaction states"),
					Player->bIsDBNO && Player->DBNORevivalStacking == 0 && !Helper->ItemInteractionActor);
				TestEqual(TEXT("Leaving revive range leaves downed health unchanged"), Player->GetHealth(), 60.0f);
				TestFalse(TEXT("A revive cannot start outside range"), FortAthenaHealth::TryStartRevive(Reviver, Player));
				Player->SetActorLocation(FVector(175, 0, 12000));
				TestTrue(TEXT("Returning to range allows another revive"), FortAthenaHealth::TryStartRevive(Reviver, Player));
				Reviver->UnPossess();
				TestNull(TEXT("Unpossessing immediately clears the reviver's target"), Helper->ItemInteractionActor);
				Reviver->Possess(Helper);
				Helper->SetPlayerState(CastChecked<AFortPlayerStateAthena>(Fixtures[4].Get()));
			}
		}), 0.4f));
		ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(([this, Fixtures]()
		{
			AFortPlayerPawnAthena* Player = Cast<AFortPlayerPawnAthena>(Fixtures[3].Get());
			AController* Reviver = Cast<AController>(Fixtures[0].Get());
			if (Player && Reviver)
			{
				TestTrue(TEXT("Repossessing before the timer ticks still cancels the revive"),
					Player->bIsDBNO && Player->DBNORevivalStacking == 0);
				TestEqual(TEXT("The cancelled revive leaves downed health unchanged"), Player->GetHealth(), 60.0f);
				Player->SetMaxHealth(20.0f);
				TestTrue(TEXT("The reviver can restart after repossessing"), FortAthenaHealth::TryStartRevive(Reviver, Player));
			}
		}), 0.4f));
	}
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(([this, Fixtures]()
	{
		AFortPlayerPawnAthena* Helper = Cast<AFortPlayerPawnAthena>(Fixtures[2].Get());
		AFortPlayerPawnAthena* Player = Cast<AFortPlayerPawnAthena>(Fixtures[3].Get());
		if (Helper && Player && Fixtures[0].IsValid() && Fixtures[1].IsValid() && Fixtures[4].IsValid() && Fixtures[5].IsValid())
		{
			TestFalse(TEXT("A completed revive clears the downed state"), Player->bIsDBNO);
			TestEqual(TEXT("A completed revive respects the standing health cap"), Player->GetHealth(), 20.0f);
			TestTrue(TEXT("A completed revive clears both interaction states"),
				!Helper->ItemInteractionActor && Player->DBNORevivalStacking == 0);
			AController* Reviver = CastChecked<AController>(Fixtures[0].Get());
			Player->bIsDBNO = true;
			if (TestTrue(TEXT("The helper can pick up the downed player again"), FortAthenaHealth::TryCarryDownedPlayer(Reviver, Player)))
			{
				Reviver->UnPossess();
				TestTrue(TEXT("Unpossessing the carrier releases the downed player immediately"),
					!Helper->DBNOHoistee && !Player->GetAttachParentActor() &&
					!Player->bIsBeingDBNOCarried && Player->GetCharacterMovement()->IsFalling());
			}
		}
		for (const TWeakObjectPtr<AActor>& Fixture : Fixtures)
		{
			if (AActor* Actor = Fixture.Get())
			{
				Actor->Destroy();
			}
		}
	}), 0.4f));

	AFortPawn* Pawn = World->SpawnActor<AFortPawn>(FVector(0, 0, 10000), FRotator::ZeroRotator, SpawnParameters);
	if (!TestNotNull(TEXT("Damage test pawn"), Pawn))
	{
		return false;
	}

	UFortAbilitySystemComponent* ASC = NewObject<UFortAbilitySystemComponent>(Pawn);
	Pawn->AbilitySystemComponent = ASC;
	ASC->RegisterComponent();
	ASC->InitAbilityActorInfo(Pawn, Pawn);
	ASC->AddAttributeSetSubobject(NewObject<UFortHealthSet>(Pawn));
	Pawn->bHealthSynced = false;

	const FGameplayAttribute Health(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Health")));
	const FGameplayAttribute Shield(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("CurrentShield")));
	ASC->SetNumericAttributeBase(Health, 100);
	ASC->SetNumericAttributeBase(Shield, 30);

	UServerActorDamageListener::Bind(*Pawn);
	UServerActorDamageListener::Bind(*Pawn);
	AFortPawn* Attacker = World->SpawnActor<AFortPawn>(FVector(100, 0, 10000), FRotator::ZeroRotator, SpawnParameters);
	if (!TestNotNull(TEXT("Attacking pawn"), Attacker))
	{
		Pawn->Destroy();
		return false;
	}
	UFortAbilitySystemComponent* AttackerASC = NewObject<UFortAbilitySystemComponent>(Attacker);
	Attacker->AbilitySystemComponent = AttackerASC;
	AttackerASC->RegisterComponent();
	AttackerASC->InitAbilityActorInfo(Attacker, Attacker);

	// A pawn can directly cause damage before it has a controller.
	FGameplayEffectContextHandle HitContext;
	auto& ShieldChanged = ASC->GetGameplayAttributeValueChangeDelegate(Shield);
	const FDelegateHandle HitListener = ShieldChanged.AddLambda([&HitContext](const FOnAttributeChangeData& Change)
	{
		if (Change.GEModData)
		{
			HitContext = Change.GEModData->EffectSpec.GetContext();
		}
	});
	Pawn->TakeDamage(15, FPointDamageEvent(), nullptr, Attacker);
	ShieldChanged.Remove(HitListener);
	TestTrue(TEXT("First hit synchronizes the non-player pawn health set"), Pawn->bHealthSynced);
	TestTrue(TEXT("Damage captures the attacking ability system"), HitContext.GetInstigatorAbilitySystemComponent() == AttackerASC);
	TestTrue(TEXT("Damage retains the engine causer"), HitContext.GetEffectCauser() == Attacker);
	Attacker->Destroy();
	TestEqual(TEXT("A point damage event fires once and shield absorbs the hit"), Pawn->GetShield(), 15.0f);
	TestEqual(TEXT("Health is intact"), Pawn->GetHealth(), 100.0f);

	Pawn->SetCanBeDamaged(false);
	Pawn->TakeDamage(40, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("Disabled engine damage preserves shield"), Pawn->GetShield(), 15.0f);
	TestEqual(TEXT("Disabled engine damage preserves health"), Pawn->GetHealth(), 100.0f);
	Pawn->SetCanBeDamaged(true);

	FRadialDamageEvent Blast;
	Blast.Origin = Pawn->GetActorLocation() - FVector(100, 0, 0);
	Blast.Params.BaseDamage = 80.0f;
	Blast.Params.MinimumDamage = 0.0f;
	Blast.Params.InnerRadius = 0.0f;
	Blast.Params.OuterRadius = 200.0f;
	Blast.Params.DamageFalloff = 1.0f;
	Blast.ComponentHits.AddDefaulted_GetRef().ImpactPoint = Pawn->GetActorLocation();
	TestEqual(TEXT("Engine radial falloff resolves halfway to 40"), Pawn->TakeDamage(80, Blast, nullptr, nullptr), 40.0f);
	TestEqual(TEXT("Radial damage depletes the remaining shield"), Pawn->GetShield(), 0.0f);
	TestEqual(TEXT("Only the radial overflow reduces health"), Pawn->GetHealth(), 75.0f);

	UServerDamageEffect::BindDamageAttribute(*ASC);
	const FGameplayAttribute Damage(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Damage")));
	UGameplayEffect* AuthoredDamage = NewObject<UGameplayEffect>(GetTransientPackage());
	FGameplayModifierInfo& DamageModifier = AuthoredDamage->Modifiers.AddDefaulted_GetRef();
	DamageModifier.Attribute = Damage;
	DamageModifier.ModifierOp = EGameplayModOp::Additive;
	DamageModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(10.0f));
	FGameplayEffectSpec DamageSpec(AuthoredDamage, ASC->MakeEffectContext(), 1.0f);
	ASC->SetNumericAttributeBase(Shield, 15);
	ASC->ApplyGameplayEffectSpecToSelf(DamageSpec);
	TestEqual(TEXT("Authored Damage drains shield"), Pawn->GetShield(), 5.0f);
	TestEqual(TEXT("Authored Damage preserves health while shield remains"), Pawn->GetHealth(), 75.0f);
	TestEqual(TEXT("Damage accumulator clears after each effect"), ASC->GetNumericAttribute(Damage), 0.0f);

	const FGameplayAttribute ShieldDamage(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("ShieldDamage")));
	UGameplayEffect* AuthoredShieldDamage = NewObject<UGameplayEffect>(GetTransientPackage());
	AuthoredShieldDamage->Modifiers.Add(DamageModifier);
	AuthoredShieldDamage->Modifiers.Last().Attribute = ShieldDamage;
	FGameplayEffectSpec ShieldDamageSpec(AuthoredShieldDamage, ASC->MakeEffectContext(), 1.0f);
	ASC->ApplyGameplayEffectSpecToSelf(ShieldDamageSpec);
	TestEqual(TEXT("Authored ShieldDamage drains remaining shield"), Pawn->GetShield(), 0.0f);
	TestEqual(TEXT("Authored ShieldDamage has no health overflow"), Pawn->GetHealth(), 75.0f);
	TestEqual(TEXT("ShieldDamage accumulator clears after each effect"), ASC->GetNumericAttribute(ShieldDamage), 0.0f);

	UGameplayEffect* FormulaEffect = NewObject<UGameplayEffect>(GetTransientPackage());
	FormulaEffect->Executions.AddDefaulted_GetRef().CalculationClass = UFortDamageFormulaExecutionCalculation::StaticClass();
	FGameplayEffectSpec FormulaSpec(FormulaEffect, ASC->MakeEffectContext(), 1.0f);
	FormulaSpec.SetSetByCallerMagnitude(FName(TEXT("Damage")), 12.0f);
	ASC->SetNumericAttributeBase(Shield, 20.0f);
	ASC->ApplyGameplayEffectSpecToSelf(FormulaSpec);
	TestEqual(TEXT("Native execution emits explicit damage into the health set"), Pawn->GetShield(), 8.0f);

	UClass* BarrelEffectClass = LoadClass<UGameplayEffect>(nullptr,
		TEXT("/Game/Creative/Devices/ExplosiveBarrel/GE_CustomDamage_ExplosiveBarrel.GE_CustomDamage_ExplosiveBarrel_C"));
	const FGameplayTag AbilityDamageTag = FGameplayTag::RequestGameplayTag(FName(TEXT("SetByCaller.AbilityDamage")), false);
	if (!TestNotNull(TEXT("Staged explosive barrel effect"), BarrelEffectClass) ||
		!TestTrue(TEXT("Authored ability damage tag is registered"), AbilityDamageTag.IsValid()))
	{
		Pawn->Destroy();
		return false;
	}
	FGameplayEffectSpec BarrelDamage(GetDefault<UGameplayEffect>(BarrelEffectClass), ASC->MakeEffectContext(), 1.0f);
	BarrelDamage.SetSetByCallerMagnitude(AbilityDamageTag, 6.0f);
	ASC->ApplyGameplayEffectSpecToSelf(BarrelDamage);
	TestEqual(TEXT("Cooked explosive barrel effect consumes shield"), Pawn->GetShield(), 2.0f);
	TestEqual(TEXT("Cooked explosive barrel effect preserves health while shield remains"), Pawn->GetHealth(), 75.0f);

	ASC->AddAttributeSetSubobject(NewObject<UFortDamageSet>(Pawn));
	const FGameplayAttribute BaseDamage(FindFProperty<FProperty>(UFortDamageSet::StaticClass(), TEXT("OutgoingBaseDamage")));
	ASC->SetNumericAttributeBase(BaseDamage, 7.0f);
	FGameplayEffectSpec CapturedDamage(FormulaEffect, ASC->MakeEffectContext(), 1.0f);
	ASC->SetNumericAttributeBase(BaseDamage, 18.0f);
	ASC->SetNumericAttributeBase(Shield, 20.0f);
	ASC->ApplyGameplayEffectSpecToSelf(CapturedDamage);
	TestEqual(TEXT("Implicit damage uses the source snapshot"), Pawn->GetShield(), 13.0f);

	FGameplayEffectSpec ZeroDamage(FormulaEffect, ASC->MakeEffectContext(), 1.0f);
	ZeroDamage.SetSetByCallerMagnitude(FName(TEXT("Damage")), 0.0f);
	ASC->ApplyGameplayEffectSpecToSelf(ZeroDamage);
	TestEqual(TEXT("Explicit zero suppresses source base damage"), Pawn->GetShield(), 13.0f);

	const FGameplayAttribute Resistance(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("DamageResistance")));
	FGameplayEffectSpec ResistedDamage(FormulaEffect, ASC->MakeEffectContext(), 1.0f);
	ResistedDamage.SetSetByCallerMagnitude(FName(TEXT("Damage")), 8.0f);
	ASC->SetNumericAttributeBase(Resistance, 0.5f);
	ASC->ApplyGameplayEffectSpecToSelf(ResistedDamage);
	TestEqual(TEXT("Target resistance applies at hit time"), Pawn->GetShield(), 9.0f);
	ASC->SetNumericAttributeBase(Resistance, 0.0f);

	FGameplayEffectSpec StackedDamage(FormulaEffect, ASC->MakeEffectContext(), 1.0f);
	StackedDamage.SetSetByCallerMagnitude(FName(TEXT("Damage")), 4.0f);
	StackedDamage.StackCount = 3;
	ASC->SetNumericAttributeBase(Shield, 8.0f);
	ASC->ApplyGameplayEffectSpecToSelf(StackedDamage);
	TestEqual(TEXT("Three damage stacks deplete eight shield"), Pawn->GetShield(), 0.0f);
	TestEqual(TEXT("Only four stacked damage overflows into health"), Pawn->GetHealth(), 71.0f);
	ASC->SetNumericAttributeBase(Health, 75.0f);

	ASC->SetNumericAttributeBase(Shield, 30);
	FortAthenaHealth::ApplyDamage(Pawn, 5, FGameplayTagContainer(), nullptr, nullptr, true);
	TestEqual(TEXT("Bypass preserves shield"), Pawn->GetShield(), 30.0f);
	TestEqual(TEXT("Bypass reduces health"), Pawn->GetHealth(), 70.0f);

	FGameplayEffectSpec AuthoredBypass(FormulaEffect, ASC->MakeEffectContext(), 1.0f);
	AuthoredBypass.SetSetByCallerMagnitude(FName(TEXT("Damage")), 6.0f);
	AuthoredBypass.SetSetByCallerMagnitude(FName(TEXT("IgnoreShield")), 1.0f);
	ASC->ApplyGameplayEffectSpecToSelf(AuthoredBypass);
	TestEqual(TEXT("Authored bypass preserves shield"), Pawn->GetShield(), 30.0f);
	TestEqual(TEXT("Authored bypass applies damage to health"), Pawn->GetHealth(), 64.0f);
	ASC->SetNumericAttributeBase(Health, 70.0f);

	ASC->SetNumericAttributeBase(FGameplayAttribute(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("MaxHealth"))), 100);
	ASC->SetNumericAttributeBase(FGameplayAttribute(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Shield"))), 100);
	UServerRecoveryEffect::BindHealingAttribute(*ASC);
	const FGameplayAttribute Healing(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Healing")));
	UGameplayEffect* AuthoredHealing = NewObject<UGameplayEffect>(GetTransientPackage());
	AuthoredHealing->DurationPolicy = EGameplayEffectDurationType::Instant;
	FGameplayModifierInfo& HealingModifier = AuthoredHealing->Modifiers.AddDefaulted_GetRef();
	HealingModifier.Attribute = Healing;
	HealingModifier.ModifierOp = EGameplayModOp::Additive;
	HealingModifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(15.0f));
	FGameplayEffectSpec HealingSpec(AuthoredHealing, ASC->MakeEffectContext(), 1.0f);
	ASC->ApplyGameplayEffectSpecToSelf(HealingSpec);
	TestEqual(TEXT("An authored Healing modifier restores health"), Pawn->GetHealth(), 85.0f);
	TestEqual(TEXT("Healing accumulator clears after each effect"), ASC->GetNumericAttribute(Healing), 0.0f);
	FGameplayEffectSpec Recovery(GetDefault<UServerRecoveryEffect>(), ASC->MakeEffectContext(), 1.0f);
	Recovery.SetSetByCallerMagnitude(FName(TEXT("RestoreShield")), 25.0f);
	Recovery.SetSetByCallerMagnitude(FName(TEXT("ShieldCap")), 50.0f);
	Recovery.SetSetByCallerMagnitude(FName(TEXT("RestoreHealth")), 50.0f);
	Pawn->bIsDBNO = true;
	ASC->ApplyGameplayEffectSpecToSelf(HealingSpec);
	ASC->ApplyGameplayEffectSpecToSelf(Recovery);
	TestEqual(TEXT("Recovery cannot refill downed health"), Pawn->GetHealth(), 85.0f);
	TestEqual(TEXT("Recovery cannot add shield while downed"), Pawn->GetShield(), 30.0f);
	TestEqual(TEXT("Blocked downed healing still clears the accumulator"), ASC->GetNumericAttribute(Healing), 0.0f);
	Pawn->bIsDBNO = false;

	ASC->ApplyGameplayEffectSpecToSelf(Recovery);
	TestEqual(TEXT("Shield recovery respects the item cap"), Pawn->GetShield(), 50.0f);
	TestEqual(TEXT("Health recovery respects the pawn maximum"), Pawn->GetHealth(), 100.0f);
	ASC->ApplyGameplayEffectSpecToSelf(Recovery);
	TestEqual(TEXT("Recovery at the shield cap changes nothing"), Pawn->GetShield(), 50.0f);

	UClass* BandageClass = LoadClass<UGameplayEffect>(nullptr,
		TEXT("/Game/Athena/Items/Consumables/Bandage/GE_Athena_Bandage_Heal.GE_Athena_Bandage_Heal_C"));
	UClass* MedkitClass = LoadClass<UGameplayEffect>(nullptr,
		TEXT("/Game/Athena/Items/Consumables/Medkit/GE_Athena_Medkit_Heal.GE_Athena_Medkit_Heal_C"));
	if (!TestNotNull(TEXT("Staged bandage effect"), BandageClass) ||
		!TestNotNull(TEXT("Staged medkit effect"), MedkitClass))
	{
		Pawn->Destroy();
		return false;
	}

	ASC->SetNumericAttributeBase(Health, 50.0f);
	FGameplayEffectSpec Bandage(GetDefault<UGameplayEffect>(BandageClass), ASC->MakeEffectContext(), 1.0f);
	ASC->ApplyGameplayEffectSpecToSelf(Bandage);
	TestTrue(TEXT("Direct cooked bandage effect reaches health recovery"), Pawn->GetHealth() > 50.0f);
	const float HealthAfterBandage = Pawn->GetHealth();
	FGameplayEffectSpec Medkit(GetDefault<UGameplayEffect>(MedkitClass), ASC->MakeEffectContext(), 1.0f);
	ASC->ApplyGameplayEffectSpecToSelf(Medkit);
	TestTrue(TEXT("Direct cooked medkit effect reaches health recovery"), Pawn->GetHealth() > HealthAfterBandage);
	TestEqual(TEXT("Cooked effects clear the healing accumulator"), ASC->GetNumericAttribute(Healing), 0.0f);

	UClass* ParentEffectClass = LoadClass<UGameplayEffect>(nullptr,
		TEXT("/Game/Athena/Items/Consumables/Parents/GE_Athena_MedConsumable_Parent.GE_Athena_MedConsumable_Parent_C"));
	if (!TestNotNull(TEXT("Staged shared consumable effect"), ParentEffectClass))
	{
		Pawn->Destroy();
		return false;
	}
	ASC->SetNumericAttributeBase(Health, 50.0f);
	ASC->SetNumericAttributeBase(Shield, 30.0f);
	FGameplayEffectSpec ParentEffect(GetDefault<UGameplayEffect>(ParentEffectClass), ASC->MakeEffectContext(), 1.0f);
	ParentEffect.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName(TEXT("SetByCaller.Healing"))), 15.0f);
	ParentEffect.SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName(TEXT("SetByCaller.Shield"))), 20.0f);
	ASC->ApplyGameplayEffectSpecToSelf(ParentEffect);
	TestEqual(TEXT("Shared cooked effect reads the healing magnitude"), Pawn->GetHealth(), 65.0f);
	TestEqual(TEXT("Shared cooked effect reads the shield magnitude"), Pawn->GetShield(), 50.0f);

	Pawn->TakeDamage(150, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("Lethal damage clamps health to zero"), Pawn->GetHealth(), 0.0f);
	TestEqual(TEXT("Lethal damage depletes the remaining shield"), Pawn->GetShield(), 0.0f);
	TestTrue(TEXT("A non-player pawn enters the dying state"), Pawn->bIsDying != 0);
	TestFalse(TEXT("A dying pawn stops accepting damage events"), Pawn->CanBeDamaged());

	Pawn->Destroy();

	AFortAIPawn* AI = World->SpawnActor<AFortAIPawn>(FVector(0, 0, 10000), FRotator::ZeroRotator, SpawnParameters);
	if (!TestNotNull(TEXT("Spawned AI damage target"), AI))
	{
		return false;
	}
	UAbilitySystemComponent* AIASC = AI->GetAbilitySystemComponent();
	if (!TestNotNull(TEXT("AI native ability system"), AIASC))
	{
		AI->Destroy();
		return false;
	}
	AIASC->AddAttributeSetSubobject(NewObject<UFortHealthSet>(AI));
	AIASC->SetNumericAttributeBase(Health, 50.0f);
	AI->SetCanBeDamaged(true);
	AI->TakeDamage(10, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("Spawned AI receives engine damage without explicit listener binding"), AI->GetHealth(), 40.0f);
	AI->Destroy();

	ABuildingWall* Wall = World->SpawnActor<ABuildingWall>(FVector(0, 0, 10000), FRotator::ZeroRotator, SpawnParameters);
	if (!TestNotNull(TEXT("Damage test wall"), Wall))
	{
		return false;
	}
	Wall->MaxHealthInitializationValue = 100.0f;
	Wall->bIsInvulnerable = false;
	Wall->SetCanBeDamaged(false);
	FortHarvest::BindLevelDamageEvents(Wall->GetLevel());
	Wall->TakeDamage(20, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("Disabled engine damage preserves structure health"), FortHarvest::GetBuildingHealth(Wall), 100.0f);

	Wall->SetCanBeDamaged(true);
	Wall->TakeDamage(20, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("A previously disabled structure takes damage when enabled"), FortHarvest::GetBuildingHealth(Wall), 80.0f);
	Wall->bIsInvulnerable = true;
	Wall->TakeDamage(20, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("Invulnerability blocks engine damage"), FortHarvest::GetBuildingHealth(Wall), 80.0f);
	Wall->Destroy();
	AFortGameModeAthena* Mode = Cast<AFortGameModeAthena>(World->GetAuthGameMode());
	AFortGameStateAthena* State = World->GetGameState<AFortGameStateAthena>();
	if (!TestNotNull(TEXT("Team knock fixture game mode"), Mode) || !TestNotNull(TEXT("Team knock fixture game state"), State))
	{
		return false;
	}

	AFortPlayerControllerAthena* First = World->SpawnActor<AFortPlayerControllerAthena>();
	AFortPlayerControllerAthena* Second = World->SpawnActor<AFortPlayerControllerAthena>();
	AFortPlayerControllerAthena* Third = World->SpawnActor<AFortPlayerControllerAthena>();
	AFortPlayerControllerAthena* NoKnockController = World->SpawnActor<AFortPlayerControllerAthena>();
	AFortPlayerPawnAthena* FirstPawn = World->SpawnActor<AFortPlayerPawnAthena>(
		FVector(0, 0, 14000), FRotator::ZeroRotator, SpawnParameters);
	AFortPlayerPawnAthena* SecondPawn = World->SpawnActor<AFortPlayerPawnAthena>(
		FVector(200, 0, 14000), FRotator::ZeroRotator, SpawnParameters);
	AFortPlayerPawnAthena* ThirdPawn = World->SpawnActor<AFortPlayerPawnAthena>(
		FVector(400, 0, 14000), FRotator::ZeroRotator, SpawnParameters);
	AFortPlayerPawnAthena* NoKnockPawn = World->SpawnActor<AFortPlayerPawnAthena>(
		FVector(600, 0, 14000), FRotator::ZeroRotator, SpawnParameters);
	if (!TestNotNull(TEXT("First team fixture controller"), First) ||
		!TestNotNull(TEXT("Second team fixture controller"), Second) ||
		!TestNotNull(TEXT("First team fixture pawn"), FirstPawn) ||
		!TestNotNull(TEXT("Second team fixture pawn"), SecondPawn) ||
		!TestNotNull(TEXT("Third team fixture controller"), Third) ||
		!TestNotNull(TEXT("Third team fixture pawn"), ThirdPawn) ||
		!TestNotNull(TEXT("DBNO-disabled fixture controller"), NoKnockController) ||
		!TestNotNull(TEXT("DBNO-disabled fixture pawn"), NoKnockPawn))
	{
		return false;
	}

	AFortPlayerStateAthena* FirstState = Cast<AFortPlayerStateAthena>(First->PlayerState);
	AFortPlayerStateAthena* SecondState = Cast<AFortPlayerStateAthena>(Second->PlayerState);
	AFortPlayerStateAthena* ThirdState = Cast<AFortPlayerStateAthena>(Third->PlayerState);
	AFortPlayerStateAthena* NoKnockState = Cast<AFortPlayerStateAthena>(NoKnockController->PlayerState);
	if (!TestNotNull(TEXT("First team fixture player state"), FirstState) ||
		!TestNotNull(TEXT("Second team fixture player state"), SecondState) ||
		!TestNotNull(TEXT("Third team fixture player state"), ThirdState) ||
		!TestNotNull(TEXT("DBNO-disabled fixture player state"), NoKnockState))
	{
		return false;
	}

	const TWeakObjectPtr<UFortPlaylistAthena> PreviousPlaylist(State->CurrentPlaylistInfo.OverridePlaylist);
	UFortPlaylistAthena* TeamPlaylist = NewObject<UFortPlaylistAthena>(State);
	TeamPlaylist->MaxTeamSize = 4;
	State->CurrentPlaylistInfo.OverridePlaylist = TeamPlaylist;
	FirstState->TeamIndex = SecondState->TeamIndex = ThirdState->TeamIndex = NoKnockState->TeamIndex = 30;
	First->Possess(FirstPawn);
	Second->Possess(SecondPawn);
	Third->Possess(ThirdPawn);
	NoKnockController->Possess(NoKnockPawn);
	for (AFortPlayerPawnAthena* TeamPawn : {FirstPawn, SecondPawn, ThirdPawn, NoKnockPawn})
	{
		TeamPawn->GetCharacterMovement()->SetComponentTickEnabled(false);
		UFortAbilitySystemComponent* TeamASC = NewObject<UFortAbilitySystemComponent>(TeamPawn);
		TeamPawn->AbilitySystemComponent = TeamASC;
		TeamASC->RegisterComponent();
		TeamASC->InitAbilityActorInfo(TeamPawn, TeamPawn);
		TeamPawn->HealthSet = NewObject<UFortHealthSet>(TeamPawn);
		TeamASC->AddAttributeSetSubobject(TeamPawn->HealthSet);
		TeamPawn->bHealthSynced = true;
		TeamPawn->SetMaxHealth(100.0f);
		TeamPawn->SetHealth(100.0f);
	}
	FortAthenaHealth::RegisterAlivePlayer(Mode, First);
	FortAthenaHealth::RegisterAlivePlayer(Mode, Second);
	FortAthenaHealth::RegisterAlivePlayer(Mode, Third);
	FortAthenaHealth::RegisterAlivePlayer(Mode, NoKnockController);

	TeamPlaylist->DBNOType = EDBNOType::Off;
	FortAthenaHealth::ApplyDamage(NoKnockPawn, 200.0f, FGameplayTagContainer(), nullptr, nullptr, true);
	TestTrue(TEXT("A team playlist with DBNO disabled eliminates a player with standing teammates"),
		NoKnockPawn->bIsDying && !NoKnockPawn->bIsDBNO);
	UFortAbilitySystemComponent* FirstASC = FirstPawn->AbilitySystemComponent;
	const FGameplayAbilitySpecHandle RangedAbilityHandle = FirstASC->GiveAbility(
		FGameplayAbilitySpec(UFortGameplayAbility_RangedWeapon::StaticClass()));
	const UGameplayAbility* RangedAbility = GetDefault<UFortGameplayAbility_RangedWeapon>();
	TestTrue(TEXT("A standing player can activate a ranged ability"),
		RangedAbility->CanActivateAbility(RangedAbilityHandle, FirstASC->AbilityActorInfo.Get()));
	{
		// Let the headless server start the predicted ability through GAS's authority-only path.
		TGuardValue<TWeakObjectPtr<APlayerController>> LocalActivation(
			FirstASC->AbilityActorInfo->PlayerController, TWeakObjectPtr<APlayerController>());
		TestTrue(TEXT("The ranged ability starts before knock"), FirstASC->TryActivateAbility(RangedAbilityHandle, false));
	}
	const FGameplayAbilitySpec* RangedSpec = FirstASC->FindAbilitySpecFromHandle(RangedAbilityHandle);
	TestTrue(TEXT("The ranged ability is still active before knock"), RangedSpec && RangedSpec->IsActive());

	TeamPlaylist->DBNOType = EDBNOType::NotWhenRespawning;
	TeamPlaylist->RespawnType = EAthenaRespawnType::InfiniteRespawnExceptStorm;
	const FGameplayTag StormTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x360);
	TestTrue(TEXT("Storm damage tag is available"), StormTag.IsValid());
	FortAthenaHealth::ApplyDamage(FirstPawn, 200.0f, FGameplayTagContainer(StormTag), nullptr, nullptr, true);
	TestTrue(TEXT("Lethal storm damage knocks a player when the playlist excludes storm respawns"),
		FirstPawn->bIsDBNO && !FirstPawn->bIsDying);
	TeamPlaylist->DBNOType = EDBNOType::On;
	TeamPlaylist->RespawnType = EAthenaRespawnType::None;
	TestEqual(TEXT("A knock fills the downed health pool"), FirstPawn->GetHealth(), 100.0f);
	TestFalse(TEXT("A downed player cannot activate a ranged ability"),
		RangedAbility->CanActivateAbility(RangedAbilityHandle, FirstASC->AbilityActorInfo.Get()));
	RangedSpec = FirstASC->FindAbilitySpecFromHandle(RangedAbilityHandle);
	TestTrue(TEXT("Knock cancels the active ranged ability"), RangedSpec && !RangedSpec->IsActive());
	TestTrue(TEXT("A knock queues the crawl stance immediately"), FirstPawn->GetCharacterMovement()->bWantsToCrouch != 0);
	SecondPawn->bAllowDBNOCarry = true;
	TestTrue(TEXT("A standing teammate can carry the knocked player"),
		FortAthenaHealth::TryCarryDownedPlayer(Second, FirstPawn));

	const TSharedRef<TOptional<float>> HealthAfterFirstBleed = MakeShared<TOptional<float>>();
	const FGameplayAttribute HealthAttribute(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Health")));
	FirstPawn->AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(HealthAttribute).AddLambda(
		[HealthAfterFirstBleed](const FOnAttributeChangeData& Change)
		{
			if (!HealthAfterFirstBleed->IsSet() && Change.NewValue < Change.OldValue)
			{
				*HealthAfterFirstBleed = Change.NewValue;
			}
		});

	const TWeakObjectPtr<AFortPlayerPawnAthena> WeakFirstPawn(FirstPawn);
	const TWeakObjectPtr<AFortPlayerPawnAthena> WeakSecondPawn(SecondPawn);
	const TWeakObjectPtr<AFortPlayerPawnAthena> WeakThirdPawn(ThirdPawn);
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(([this, WeakFirstPawn, WeakSecondPawn, WeakThirdPawn, HealthAfterFirstBleed]()
	{
		AFortPlayerPawnAthena* DownedPawn = WeakFirstPawn.Get();
		AFortPlayerPawnAthena* CarrierPawn = WeakSecondPawn.Get();
		AFortPlayerPawnAthena* StandingPawn = WeakThirdPawn.Get();
		if (TestNotNull(TEXT("Downed team fixture stays alive through first bleed tick"), DownedPawn) &&
			TestNotNull(TEXT("Carrier survives until the lethal hit"), CarrierPawn) &&
			TestNotNull(TEXT("Standing team fixture survives until the lethal hit"), StandingPawn))
		{
			TestTrue(TEXT("Bleed ticks drain downed health while a teammate stands"),
				DownedPawn->bIsDBNO && !DownedPawn->bIsDying);
			TestEqual(TEXT("The first bleed tick removes two downed health"), HealthAfterFirstBleed->Get(0.0f), 98.0f);
			TestTrue(TEXT("Bleed continues while the downed player is carried"),
				DownedPawn->GetAttachParentActor() == CarrierPawn && DownedPawn->bIsBeingDBNOCarried);
			FortAthenaHealth::ApplyDamage(CarrierPawn, 200.0f, FGameplayTagContainer(), nullptr, nullptr, true);
			TestTrue(TEXT("A carrier with a standing teammate is knocked"), CarrierPawn->bIsDBNO && !CarrierPawn->bIsDying);
			TestTrue(TEXT("Knocking the carrier immediately drops the downed player"),
				!CarrierPawn->DBNOHoistee && !CarrierPawn->bIsDBNOCarrying &&
				!DownedPawn->DBNOHoisterData.DBNOHoister && !DownedPawn->bIsBeingDBNOCarried &&
				DownedPawn->DBNOHoisterData.DBNOCarryEvent == EFortDBNOCarryEvent::Dropped &&
				!DownedPawn->GetAttachParentActor() && DownedPawn->GetCharacterMovement()->IsFalling());
			StandingPawn->bAllowDBNOCarry = true;
			TestTrue(TEXT("The last standing teammate can pick up the released player"),
				FortAthenaHealth::TryCarryDownedPlayer(StandingPawn->GetController(), DownedPawn));
			FortAthenaHealth::ApplyDamage(StandingPawn, 200.0f, FGameplayTagContainer(), nullptr, nullptr, true);
			TestTrue(TEXT("The last standing member is eliminated without a knock"), StandingPawn->bIsDying && !StandingPawn->bIsDBNO);
			TestTrue(TEXT("Eliminating the carrier releases the downed player and restores falling"),
				!StandingPawn->DBNOHoistee && !StandingPawn->bIsDBNOCarrying &&
				!DownedPawn->DBNOHoisterData.DBNOHoister && !DownedPawn->bIsBeingDBNOCarried &&
				DownedPawn->DBNOHoisterData.DBNOCarryEvent == EFortDBNOCarryEvent::Dropped &&
				!DownedPawn->GetAttachParentActor() && DownedPawn->GetCharacterMovement()->IsFalling());
		}
	}), 1.5f));

	const TWeakObjectPtr<AFortGameStateAthena> WeakState(State);
	const TArray<TWeakObjectPtr<AActor>> TeamFixtures = {First, Second, Third, NoKnockController, FirstPawn, SecondPawn, ThirdPawn, NoKnockPawn};
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand(([this, WeakFirstPawn, WeakState, PreviousPlaylist, TeamFixtures]()
	{
		AFortPlayerPawnAthena* DownedPawn = WeakFirstPawn.Get();
		if (TestNotNull(TEXT("Downed team fixture after last teammate elimination"), DownedPawn))
		{
			TestTrue(TEXT("The next bleed tick finishes the downed squad member"), DownedPawn->bIsDying);
		}
		if (AFortGameStateAthena* CurrentState = WeakState.Get())
		{
			CurrentState->CurrentPlaylistInfo.OverridePlaylist = PreviousPlaylist.Get();
		}
		for (const TWeakObjectPtr<AActor>& Fixture : TeamFixtures)
		{
			if (AActor* Actor = Fixture.Get())
			{
				Actor->Destroy();
			}
		}
	}), 1.5f));

	return true;
}

#endif
