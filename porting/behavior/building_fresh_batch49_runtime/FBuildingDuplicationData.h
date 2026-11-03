#pragma once
#include "CoreMinimal.h"
#include "FBuildingDuplicationData.generated.h"

class UBuildingTextureData;

// Reflected layout: size 0x28, class at 0, four texture pointers at 8.
USTRUCT()
struct FBuildingDuplicationData
{
    GENERATED_BODY()
    UPROPERTY() UClass* ClassData = nullptr;
    UPROPERTY() UBuildingTextureData* TextureData[4] = {};
};
