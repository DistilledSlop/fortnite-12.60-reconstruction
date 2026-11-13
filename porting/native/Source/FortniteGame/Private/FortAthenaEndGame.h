#pragma once

#include "CoreMinimal.h"

class AFortGameModeAthena;
class AFortPlayerControllerAthena;
class AFortPawn;
class UFortWeaponItemDefinition;
enum class EDeathCause : uint8;

namespace FortAthenaEndGame
{
    bool IsEnabled();
    // Called after Health has removed the victim from its alive list and reported victim placement.
    void OnPlayerEliminated(AFortGameModeAthena* GameMode, int32 OldPlayersLeft,
        AFortPlayerControllerAthena* Survivor, AFortPawn* FinisherPawn,
        UFortWeaponItemDefinition* FinishingWeapon, EDeathCause DeathCause);
    // Called by MatchFlow's existing driver. QuickEndGame is a solo-only test stand-in.
    void Tick(AFortGameModeAthena* GameMode, float DeltaSeconds);
}
