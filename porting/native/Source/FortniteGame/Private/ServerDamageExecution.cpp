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

UServerRecoveryEffect::UServerRecoveryEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	Executions.AddDefaulted_GetRef().CalculationClass = UServerRecoveryExecution::StaticClass();
}

void UServerRecoveryEffect::BindHealingAttribute(UAbilitySystemComponent& ASC)
{
	static const FGameplayAttribute Healing(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Healing")));
	TWeakObjectPtr<UAbilitySystemComponent> WeakASC(&ASC);
	ASC.GetGameplayAttributeValueChangeDelegate(Healing).AddLambda([WeakASC](const FOnAttributeChangeData& Change)
	{
		UAbilitySystemComponent* Target = WeakASC.Get();
		const AFortPawn* Pawn = Target ? Cast<AFortPawn>(Target->GetAvatarActor()) : nullptr;
		const float Amount = Change.NewValue - Change.OldValue;
		if (!Pawn || !Pawn->HasAuthority() || !Change.GEModData || !FMath::IsFinite(Amount) || Amount <= 0.0f)
		{
			return;
		}

		// Healing is an accumulator used by instant gameplay effects. Clear it
		// before applying recovery so the next effect starts from zero.
		Target->SetNumericAttributeBase(Healing, 0.0f);
		FGameplayEffectSpec Recovery(GetDefault<UServerRecoveryEffect>(), Target->MakeEffectContext(), 1.0f);
		Recovery.SetSetByCallerMagnitude(FName(TEXT("RestoreHealth")), Amount);
		Target->ApplyGameplayEffectSpecToSelf(Recovery);
	});
}

void UServerRecoveryExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& Parameters,
	FGameplayEffectCustomExecutionOutput& Output) const
{
	UAbilitySystemComponent* Target = Parameters.GetTargetAbilitySystemComponent();
	const AFortPawn* Pawn = Target ? Cast<AFortPawn>(Target->GetAvatarActor()) : nullptr;
	if (!Pawn || !Pawn->HasAuthority() || Pawn->bIsDying)
	{
		return;
	}

	static const FGameplayAttribute Health(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Health")));
	static const FGameplayAttribute MaxHealth(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("MaxHealth")));
	static const FGameplayAttribute Shield(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("CurrentShield")));
	static const FGameplayAttribute MaxShield(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Shield")));
	if (!Target->HasAttributeSetForAttribute(Health) || Target->GetNumericAttribute(Health) <= 0.0f)
	{
		return;
	}

	const FGameplayEffectSpec& Spec = Parameters.GetOwningSpec();
	const auto AddRecovery = [&](const FGameplayAttribute& Attribute, const FGameplayAttribute& Maximum,
		FName AmountName, FName CapName)
	{
		const float Amount = Spec.GetSetByCallerMagnitude(AmountName, false);
		const float MaxValue = Target->GetNumericAttribute(Maximum);
		const float Cap = Spec.GetSetByCallerMagnitude(CapName, false, MaxValue);
		if (!FMath::IsFinite(Amount) || !FMath::IsFinite(Cap) || Amount <= 0.0f)
		{
			return;
		}

		const float Room = FMath::Min(Cap, MaxValue) - Target->GetNumericAttribute(Attribute);
		if (Room > 0.0f)
		{
			Output.AddOutputModifier(FGameplayModifierEvaluatedData(Attribute, EGameplayModOp::Additive,
				FMath::Min(Amount, Room)));
		}
	};

	AddRecovery(Health, MaxHealth, FName(TEXT("RestoreHealth")), FName(TEXT("HealthCap")));
	AddRecovery(Shield, MaxShield, FName(TEXT("RestoreShield")), FName(TEXT("ShieldCap")));
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
