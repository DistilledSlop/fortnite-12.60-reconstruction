#pragma once

#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ServerDamageExecution.generated.h"

class UDamageType;

// Supply a positive Damage SetByCaller magnitude on an instant gameplay effect.
// A positive IgnoreShield magnitude makes the effect bypass shield.
UCLASS()
class UServerDamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& Parameters,
		FGameplayEffectCustomExecutionOutput& Output) const override;
};

UCLASS()
class UServerDamageEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UServerDamageEffect();
};

// For pawns whose health is managed by the server's ability system.
UCLASS()
class UServerPawnDamageListener : public UObject
{
	GENERATED_BODY()

public:
	static void Bind(AActor& Actor);

private:
	UFUNCTION()
	void OnDamage(AActor* Actor, float Amount, const UDamageType* DamageType,
		AController* Instigator, AActor* Causer);
};
