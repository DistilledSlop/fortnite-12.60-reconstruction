#include "FortniteMapClasses.h"
#include "UFortGameInstance.h"
#include "SharedFortGameLog.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void AFortSkydivingShadowProxy::ShowShadowProxy_Implementation() {}
void AFortSkydivingShadowProxy::HideShadowProxy_Implementation() {}

// The original shipping wrappers call the shared bare-return native 0x1406801c0.

AFortSkydivingShadowProxy::AFortSkydivingShadowProxy()
{
    DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
    RootComponent = DefaultSceneRoot;
}

// Native 0x140df1140; capture evidence in utility_leaf357_runtime.

// Native 0x141f8f0a0; capture evidence in utility_leaf357_runtime.

// Native 0x141f8f570; capture evidence in utility_leaf357_runtime.

// Native 0x141ef9370; capture evidence in utility_leaf357_runtime.

// CONFIDENCE: high; native exec 0x142934740 calls the verified constant true helper.

// CONFIDENCE: high; native exec 0x142934740 calls the verified constant true helper.

// CONFIDENCE: high; native exec 0x142934680 calls the verified constant false helper.

// CONFIDENCE: high; native exec 0x142934680 calls the verified constant false helper.

// CONFIDENCE: high; native exec 0x142934680 calls the verified constant false helper.

// CONFIDENCE: high; native exec 0x142934680 calls the verified constant false helper.

