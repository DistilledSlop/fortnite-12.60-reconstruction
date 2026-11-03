#pragma once

#include "CoreMinimal.h"
#include "AnalyticsEventAttribute.h"

// Assigned partial contract for the native provider's observed event operation.
// Other provider operations and the implementation are not yet recovered.
class FFortNativeAnalyticsProvider
{
public:
    virtual ~FFortNativeAnalyticsProvider() = default;
    virtual void NativeRecordEvent(const FString& EventName,
        const TArray<FAnalyticsEventAttribute>& Attributes) = 0;
};
