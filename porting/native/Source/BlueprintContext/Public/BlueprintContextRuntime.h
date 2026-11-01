#pragma once
#include "CoreMinimal.h"
class ULocalPlayer;
class USubsystem;
class UBlueprintContextBase;

namespace BlueprintContextRuntime
{
    BLUEPRINTCONTEXT_API USubsystem* GetContext(UObject* ContextObject, TSubclassOf<USubsystem> Class);
    // Original callers: phase-finish helper14238a5c0, FortGI AddLocalPlayer141e4f680,
    // and FortLocalPlayer PlayerRemoved1423a2490. These exports do not replace them.
    BLUEPRINTCONTEXT_API void CreateContexts(ULocalPlayer* Player);
    BLUEPRINTCONTEXT_API void DestroyContexts(ULocalPlayer* Player);
    BLUEPRINTCONTEXT_API void ObserveContextInitialize(UBlueprintContextBase* Context, bool Before);
    BLUEPRINTCONTEXT_API void ObserveContextDeinitialize(UBlueprintContextBase* Context, bool Before);
    BLUEPRINTCONTEXT_API void ArmLifecycleObservation();
    BLUEPRINTCONTEXT_API bool LifecycleCallbacksObserved();
}
