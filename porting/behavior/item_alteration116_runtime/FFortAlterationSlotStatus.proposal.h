#pragma once
#include "CoreMinimal.h"
#include "EFortRarity.h"
#include "FFortAlterationSlotStatus.generated.h"

class UFortAlterationItemDefinition;

USTRUCT()
struct FFortAlterationSlotStatus
{
    GENERATED_BODY()

    UPROPERTY()
    UFortAlterationItemDefinition* Alteration;

    UPROPERTY()
    int32 MinRequiredLevel;

    UPROPERTY()
    EFortRarity MinHostItemRarity;
};
