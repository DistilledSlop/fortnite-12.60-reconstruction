// Generated native interface default implementations (porting/generated/reflection/fortnite_game_reflection.json).
#include "UFortVehicleInterface.h"
#include "AFortPlayerStateZone.h"
#include "UFortVehicleSeatComponent.h"
#include "GameFramework/Pawn.h"

bool IFortVehicleInterface::NativeCanPassengersTakeDamage163()
{
    return false;
}

bool IFortVehicleInterface::NativeContainsPlayers163()
{
    return false;
}

UFortVehicleSeatComponent* IFortVehicleInterface::NativeGetSeatComponent163()
{
    return nullptr;
}

bool IFortVehicleInterface::NativeIsSeatFilterApplicable163(uint8 Seat)
{
    return false;
}

TArray<APawn*> IFortVehicleInterface::NativeGetAllPassengers163()
{
    return {};
}

AFortPlayerStateZone* IFortVehicleInterface::NativeGetPlayerStateForPawn163(APawn* Pawn)
{
    return nullptr;
}

