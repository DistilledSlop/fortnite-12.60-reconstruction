// Generated from porting/generated/reflection/fortnite_game_reflection.json; native_virtual_interfaces_runtime policy.
#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
class APawn;
class AFortPlayerStateZone;
class UFortVehicleSeatComponent;
#include "UFortVehicleInterface.generated.h"

UINTERFACE()
class FORTNITEGAME_API UFortVehicleInterface : public UInterface
{
    GENERATED_BODY()
};

class FORTNITEGAME_API IFortVehicleInterface
{
    GENERATED_IINTERFACE_BODY()
public:
    virtual bool NativeCanPassengersTakeDamage163();
    virtual bool NativeContainsPlayers163();
    virtual UFortVehicleSeatComponent* NativeGetSeatComponent163();
    virtual bool NativeIsSeatFilterApplicable163(uint8 Seat);
    virtual TArray<APawn*> NativeGetAllPassengers163();
    virtual AFortPlayerStateZone* NativeGetPlayerStateForPawn163(APawn* Pawn);
};
