#pragma once

#include "Commandlets/Commandlet.h"
#include "FortniteBackgroundMaterialsCommandlet.generated.h"

// Builds simplified Apollo cloud and backdrop materials from recovered parameters.
UCLASS()
class UFortniteBackgroundMaterialsCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UFortniteBackgroundMaterialsCommandlet();
    virtual int32 Main(const FString& Params) override;
};
