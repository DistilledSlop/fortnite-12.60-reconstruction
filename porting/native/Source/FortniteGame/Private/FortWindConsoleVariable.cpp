#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"

// Native registration 0x1402b6440. Effects scalability controls this live value.
static TAutoConsoleVariable<int32> FortWindVariable(
    TEXT("Fort.Wind"),
    1,
    TEXT("Controls whether wind shaders and logic are enabled. Set by EffectsQuality."),
    ECVF_Scalability);

// Original console registrations 0x1402b6540 and 0x1402b64c0.
static TAutoConsoleVariable<int32> FortWindResponderQuadTreeSize(
    TEXT("Fort.WindResponderQuadTreeSize"),
    2097152,
    TEXT("The total size of the quad tree size for wind responders along either axis"),
    ECVF_Default);

static TAutoConsoleVariable<int32> FortWindResponderQuadTreeCellSize(
    TEXT("Fort.WindResponderQuadTreeCellSize"),
    2500,
    TEXT("The minimum size of a quad tree cell for wind responders"),
    ECVF_Default);
