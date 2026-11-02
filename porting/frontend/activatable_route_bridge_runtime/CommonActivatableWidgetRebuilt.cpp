#include "CommonActivatableWidgetRebuilt.h"

FCommonActivatableWidgetRebuilt& GetCommonActivatableWidgetRebuilt()
{
    static FCommonActivatableWidgetRebuilt Dispatcher;
    return Dispatcher;
}
