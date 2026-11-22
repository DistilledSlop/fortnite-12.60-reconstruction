#if WITH_DEV_AUTOMATION_TESTS

#include "ABuildingWall.h"
#include "AFortPawn.h"
#include "AFortAIPawn.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FortAthenaHealth.h"
#include "FortHarvestBuilding.h"
#include "GameFramework/DamageType.h"
#include "GameplayEffectExtension.h"
#include "Misc/AutomationTest.h"
#include "ServerDamageExecution.h"
#include "UFortAbilitySystemComponent.h"
#include "UFortDamageFormulaExecutionCalculation.h"
#include "UFortHealthSet.h"

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
	AuthoredShieldDamage->Modifiers[AuthoredShieldDamage->Modifiers.Add(DamageModifier)].Attribute = ShieldDamage;
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

	ASC->SetNumericAttributeBase(Shield, 30);
	FortAthenaHealth::ApplyDamage(Pawn, 5, FGameplayTagContainer(), nullptr, nullptr, true);
	TestEqual(TEXT("Bypass preserves shield"), Pawn->GetShield(), 30.0f);
	TestEqual(TEXT("Bypass reduces health"), Pawn->GetHealth(), 70.0f);

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
	return true;
}

#endif
