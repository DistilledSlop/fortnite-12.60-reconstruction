#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"

// Native registration 0x1402b6440. Effects scalability controls this live value.
static TAutoConsoleVariable<int32> FortWindVariable(
    TEXT("Fort.Wind"),
    1,
    TEXT("Controls whether wind shaders and logic are enabled. Set by EffectsQuality."),
    ECVF_Scalability);
