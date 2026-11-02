#pragma once
#include "UCommonInputControllerData.h"
#include "CommonInputActionRowProbe.generated.h"

UCLASS()
class UCommonInputProbeControllerFirst : public UCommonInputControllerData
{
    GENERATED_BODY()
};

UCLASS()
class UCommonInputProbeControllerSecond : public UCommonInputControllerData
{
    GENERATED_BODY()
};

// Diagnostic only. Registration is inert without -CommonInputActionRowProbe.
void RegisterCommonInputActionRowProbeDriver();
