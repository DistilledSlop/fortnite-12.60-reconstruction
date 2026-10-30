#pragma once

#include "Commandlets/Commandlet.h"
#include "FortniteApolloBlueprintCommandlet.generated.h"

// Creates editable replacement Blueprints at Apollo's original package paths.
UCLASS()
class UFortniteApolloBlueprintCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UFortniteApolloBlueprintCommandlet();
    virtual int32 Main(const FString& Params) override;
};
