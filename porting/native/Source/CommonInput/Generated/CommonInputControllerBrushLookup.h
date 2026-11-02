#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "ECommonInputType.h"
#include "ECommonGamepadType.h"
#include "Styling/SlateBrush.h"

class UCommonInputControllerData;
struct FCommonInputPlatformData;

// Source alias for the original unreflected single-key and key-set lookup path.
COMMONINPUT_API bool FindCommonInputControllerBrush(
    const UCommonInputControllerData& ControllerData, const TArray<FKey>& Keys,
    FSlateBrush& OutBrush);

// Source alias for the original ordered platform controller selection.
COMMONINPUT_API bool FindCommonInputPlatformBrush(
    const FCommonInputPlatformData& PlatformData, const TArray<FKey>& Keys,
    ECommonInputType InputType, ECommonGamepadType GamepadType, FSlateBrush& OutBrush);
