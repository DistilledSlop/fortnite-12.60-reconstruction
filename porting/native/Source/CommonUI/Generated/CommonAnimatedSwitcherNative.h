#pragma once
#include "CoreMinimal.h"
class UCommonActivatableWidget;
class UCommonActivatableWidgetQueue;
DECLARE_MULTICAST_DELEGATE_OneParam(FNativeDisplayedWidgetChanged, UCommonActivatableWidget*);
COMMONUI_API void NativeClearModalQueue1407ae7f0(UCommonActivatableWidgetQueue* Queue);
