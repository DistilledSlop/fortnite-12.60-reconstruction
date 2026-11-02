#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Styling/SlateBrush.h"

class UCommonInputControllerData;

// Source alias for the original unreflected single-key and key-set lookup path.
COMMONINPUT_API bool FindCommonInputControllerBrush(
    const UCommonInputControllerData& ControllerData, const TArray<FKey>& Keys,
    FSlateBrush& OutBrush);
