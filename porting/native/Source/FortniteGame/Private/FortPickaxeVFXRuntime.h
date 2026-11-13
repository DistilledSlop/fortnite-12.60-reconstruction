#pragma once
#include "CoreMinimal.h"
class UFXSystemComponent;
namespace FortPickaxeVFXRuntime
{
    UFXSystemComponent* GetFirstOrFallback(const UObject* Context, uint32 TagOffset, UFXSystemComponent* Fallback);
}
