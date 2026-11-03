#pragma once
#include "CoreMinimal.h"

// Partial logical representation of the actual native player loading-screen
// controller owned by UFortGameInstance. This is not its native layout or its
// complete lifecycle; recoveries of the controller must extend this same type.
struct FFortPlayerLoadingScreenState
{
    // Original object +0x40. Show/hide side effects and other members remain open.
    bool bIsShown = false;
};
