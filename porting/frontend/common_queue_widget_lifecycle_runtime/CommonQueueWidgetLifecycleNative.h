#pragma once
#include "CoreMinimal.h"
class UCommonActivatableWidget;
class UCommonActivatableWidgetQueue;
COMMONUI_API UCommonActivatableWidget* NativeAddQueueWidget1407aa4c0(UCommonActivatableWidgetQueue* Queue,
    UClass* WidgetClass, TFunctionRef<void(UCommonActivatableWidget*)> Initializer);
COMMONUI_API void NativeRemoveQueueWidget1407f1000(UCommonActivatableWidgetQueue* Queue, UCommonActivatableWidget* Widget);
