#pragma once

#include "CoreMinimal.h"

class UCommonActivatableWidget;
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonActivatableWidgetRebuilt, UCommonActivatableWidget*);

// Process-wide dispatcher corresponding to original global 14782ee30.
COMMONUI_API FCommonActivatableWidgetRebuilt& GetCommonActivatableWidgetRebuilt();
