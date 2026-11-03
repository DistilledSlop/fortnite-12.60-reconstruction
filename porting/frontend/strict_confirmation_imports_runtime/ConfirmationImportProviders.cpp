#include "ConfirmationImportProviders.h"
#include "UFortUIManagerWidget_NUI.h"
#include "UCommonUISubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

// NativeConfirmationResolveManager14514eda0 is intentionally not defined here. The previous
// body called UFortUIDirectorSubsystemBase::NativeGetConfirmationManagerPair, which does not
// exist in the native sources (that class has no members or virtuals). Provide the real
// original chain once 14514eda0's +0x150 slot target and 141e63a20 are recovered.

UCommonInputManager* NativeConfirmationFindInputManager14514f020(UObject* WorldContext)
{
    const FNativeConfirmationManagerPair* Pair = NativeConfirmationResolveManager14514eda0(WorldContext);
    UFortUIManagerWidget_NUI* ManagerWidget = Pair ? Cast<UFortUIManagerWidget_NUI>(Pair->Object) : nullptr;
    UWorld* World = ManagerWidget ? ManagerWidget->GetWorld() : nullptr;
    UGameInstance* GameInstance = World ? World->GetGameInstance() : nullptr;
    UCommonUISubsystem* Subsystem = GameInstance ? GameInstance->GetSubsystem<UCommonUISubsystem>() : nullptr;
    return Subsystem ? Subsystem->CommonInputManager : nullptr;
}

namespace { bool GNativeConfirmationIdle147c577d4 = true; }
void NativeSetConfirmationGlobalIdle147c577d4(bool bIdle) { GNativeConfirmationIdle147c577d4 = bIdle; }
bool NativeGetConfirmationGlobalIdle147c577d4() { return GNativeConfirmationIdle147c577d4; }
