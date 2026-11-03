#include "ConfirmationImportProviders.h"
#include "UCommonActivatableWidget.h"
#include "UCommonActivatableWidgetQueue.h"
#include "SCommonAnimatedSwitcher.h"
#include "Slate/SObjectWidget.h"

UCommonActivatableWidget* NativeGetQueueActiveWidget1407c1910(UCommonActivatableWidgetQueue* Queue)
{
    const TSharedPtr<SCommonAnimatedSwitcher>& Switcher = Queue->NativeAnimatedSwitcher1c8;
    if (!Switcher.IsValid())
        return nullptr;
    TSharedPtr<SWidget> Active = Switcher->GetActiveWidget();
    if (!Active.IsValid() || Active->GetType() != FName(TEXT("SObjectWidget")))
        return nullptr;
    return Cast<UCommonActivatableWidget>(StaticCastSharedPtr<SObjectWidget>(Active)->GetWidgetObject());
}
