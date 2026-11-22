#pragma once

#include "CoreMinimal.h"
#include "WeaponHudKeyActionVisibility.generated.h"

// Serialized in cooked utility-item Blueprint locals. Script struct block 0x147beecb0.
USTRUCT(BlueprintType)
struct FWeaponHudKeyActionVisibility
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Weapon HUD")
	int32 Index = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Weapon HUD")
	bool bVisibility = false;
};
