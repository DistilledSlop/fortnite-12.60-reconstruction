#pragma once

#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ServerDamageExecution.generated.h"

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
