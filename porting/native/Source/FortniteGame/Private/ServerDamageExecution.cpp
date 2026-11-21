#include "ServerDamageExecution.h"

#include "ABuildingSMActor.h"
#include "AFortPawn.h"
#include "AbilitySystemComponent.h"
#include "FortAthenaHealth.h"
#include "FortHarvestBuilding.h"
#include "GameFramework/DamageType.h"
#include "UFortHealthSet.h"

namespace
{
	struct FServerDamage
	{
		float Health = 0.0f;
		float Shield = 0.0f;

		static FServerDamage Resolve(float Amount, float Health, float Shield, bool bIgnoreShield);
	};

	FServerDamage FServerDamage::Resolve(float Amount, float Health, float Shield, bool bIgnoreShield)
	{
		FServerDamage Damage;
		if (!FMath::IsFinite(Amount) || Amount <= 0.0f)
		{
			return Damage;
		}

		Damage.Shield = bIgnoreShield ? 0.0f : FMath::Min(Amount, FMath::Max(0.0f, Shield));
		Damage.Health = FMath::Min(Amount - Damage.Shield, FMath::Max(0.0f, Health));
		return Damage;
	}
}

void UServerDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& Parameters,
	FGameplayEffectCustomExecutionOutput& Output) const
{
	UAbilitySystemComponent* Target = Parameters.GetTargetAbilitySystemComponent();
	const AFortPawn* Pawn = Target ? Cast<AFortPawn>(Target->GetAvatarActor()) : nullptr;
	if (!Pawn || !Pawn->HasAuthority() || Pawn->bIsDying)
	{
		return;
	}

	static const FGameplayAttribute Health(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Health")));
	static const FGameplayAttribute Shield(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("CurrentShield")));
	if (!Target->HasAttributeSetForAttribute(Health))
	{
		return;
	}

	const FGameplayEffectSpec& Spec = Parameters.GetOwningSpec();
	const FServerDamage Damage = FServerDamage::Resolve(
		Spec.GetSetByCallerMagnitude(FName(TEXT("Damage")), false),
		Target->GetNumericAttribute(Health), Target->GetNumericAttribute(Shield),
		Spec.GetSetByCallerMagnitude(FName(TEXT("IgnoreShield")), false) > 0.0f);
	if (Damage.Shield > 0.0f)
	{
		Output.AddOutputModifier(FGameplayModifierEvaluatedData(Shield, EGameplayModOp::Additive, -Damage.Shield));
	}
	if (Damage.Health > 0.0f)
	{
		Output.AddOutputModifier(FGameplayModifierEvaluatedData(Health, EGameplayModOp::Additive, -Damage.Health));
	}
}

UServerDamageEffect::UServerDamageEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	FGameplayEffectExecutionDefinition& Execution = Executions.AddDefaulted_GetRef();
	Execution.CalculationClass = UServerDamageExecution::StaticClass();
}

void UServerActorDamageListener::Bind(AActor& Actor)
{
	if (Actor.HasAuthority())
	{
		Actor.OnTakeAnyDamage.AddUniqueDynamic(
			GetMutableDefault<UServerActorDamageListener>(), &UServerActorDamageListener::OnDamage);
	}
}

void UServerActorDamageListener::OnDamage(AActor* Actor, float Amount, const UDamageType*,
	AController* Instigator, AActor* Causer)
{
	if (!FMath::IsFinite(Amount) || Amount <= 0.0f)
	{
		return;
	}

	if (AFortPawn* Pawn = Cast<AFortPawn>(Actor))
	{
		FortAthenaHealth::ApplyDamage(Pawn, Amount, FGameplayTagContainer(), Instigator, Causer, false);
	}
	else if (ABuildingSMActor* Building = Cast<ABuildingSMActor>(Actor))
	{
		FortHarvest::ApplyBuildingDamage(Building, Amount, Instigator, Causer, FHitResult(), false);
	}
}
