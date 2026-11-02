#pragma once

#include "CoreMinimal.h"
#include "ECommonInputMode.h"
#include "Engine/EngineBaseTypes.h"
#include "InputCoreTypes.h"

enum class ECommonActionHoldDispatchResult : uint8 { Handled = 0, GeneratePress = 1, Unhandled = 2 };

// Descriptive native names. The three bytes and their order are witnessed by
// 1407bd980 / 1407f4770; this is not an invented reflected struct.
struct FCommonActionRouteInputConfig
{
    ECommonInputMode InputMode;
    EMouseCaptureMode MouseCaptureMode;
    bool bHideCursorDuringCapture;

    bool operator==(const FCommonActionRouteInputConfig& Other) const
    {
        return InputMode == Other.InputMode && MouseCaptureMode == Other.MouseCaptureMode
            && bHideCursorDuringCapture == Other.bHideCursorDuringCapture;
    }
};
static_assert(sizeof(FCommonActionRouteInputConfig) == 3, "Original input config payload");

class UWidget;
class IInputProcessor;
// Router owner fallback +20 has the reverse field order to the node +98 entry.
struct FCommonActionRouteOwnerProcessor
{
    TSharedPtr<IInputProcessor> InputProcessor;
    int32 Index;
};
struct FCommonActionRouteOwnerRecord
{
    explicit FCommonActionRouteOwnerRecord(UWidget* InWidget);
    TWeakObjectPtr<UWidget> Widget;
    TArray<int32> BindingHandles;
    bool bRouteWidget = false;
    TArray<FCommonActionRouteOwnerProcessor> InputProcessors;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FCommonActionRouteOverrideChanged, TOptional<uint8>);
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonActionRouteModeChanged, ECommonInputMode);
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonLegacyInputManagerChanged, bool);
