// Generated from porting/generated/reflection/fortnite_game_reflection.json; native_virtual_interfaces_runtime policy.
#pragma once
#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "UFortTeamActorInterface.generated.h"

UINTERFACE()
class FORTNITEGAME_API UFortTeamActorInterface : public UGenericTeamAgentInterface
{
    GENERATED_BODY()
};

class FORTNITEGAME_API IFortTeamActorInterface : public IGenericTeamAgentInterface
{
    GENERATED_IINTERFACE_BODY()
public:
    virtual uint8 NativeGetTeam163();
};
