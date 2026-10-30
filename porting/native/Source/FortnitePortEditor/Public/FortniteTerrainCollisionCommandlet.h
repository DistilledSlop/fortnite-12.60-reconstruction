#pragma once

#include "Commandlets/Commandlet.h"
#include "FortniteTerrainCollisionCommandlet.generated.h"

// Rebuilds a converted Apollo mesh with triangle collision for walking.
UCLASS()
class UFortniteTerrainCollisionCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    UFortniteTerrainCollisionCommandlet();
    virtual int32 Main(const FString& Params) override;
};
