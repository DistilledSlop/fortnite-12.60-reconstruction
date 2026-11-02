#pragma once
#include "UCommonActivatableWidget.h"
#include "QueueConnectedFlow.generated.h"

// Concrete pool class with simple Slate content; activation remains the production implementation.
UCLASS()
class UQueueConnectedFlowWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
};

void RegisterQueueConnectedFlowDriver();
