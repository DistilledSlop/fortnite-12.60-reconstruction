#pragma once

#include "Commandlets/Commandlet.h"
#include "FortniteMaterialRebuildCommandlet.generated.h"

// Creates a minimal editable material at a recovered asset's original path.
UCLASS()
class UFortniteMaterialRebuildCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UFortniteMaterialRebuildCommandlet();
    virtual int32 Main(const FString& Params) override;
};
