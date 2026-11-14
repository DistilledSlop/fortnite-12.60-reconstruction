#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"

class UWorld;

// No replacement producer or default random values are supplied.
// Native-only state consumed by 0x141be5f10 and 0x141be60c0. Lifecycle and
// producer remain unrecovered. ValueCount is independent of Values.Num().
struct FFoundationNativeRandomSequence
{
    TArray<float> Values;
    int32 CurrentIndex;
    int32 ValueCount;

    float Next()
    {
        if (ValueCount <= 0)
            return 0.0f;
        int32 Index = CurrentIndex;
        if (Index >= ValueCount)
            Index = 0;
        CurrentIndex = Index + 1;
        // Native assumes 0 <= Index < allocated readable values.
        return Values.GetData()[Index];
    }
};

// Native element: original stride 0x38, world +0, name +0x28, weight +0x30.
struct FFoundationNativeCandidate
{
    TSoftObjectPtr<UWorld> BuildingWorld;
    FName PackageName;
    float SelectionWeight;
};

// 0x141ed80a0: soft path ToString, then remove first dot and suffix.
inline FString NativeFoundationWorldString_141ed80a0(const TSoftObjectPtr<UWorld>& World)
{
    FString Path = World.ToSoftObjectPath().ToString();
    int32 DotIndex;
    if (Path.FindChar(TEXT('.'), DotIndex))
        Path.LeftInline(DotIndex);
    return Path;
}
