#include "UCommonUISubsystem.h"
#include "UCommonInputManager.h"
#include "UCommonGlobalInputHandler.h"
#include "UCommonInputSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"

void UCommonInputManager::PostInitProperties()
{
    Super::PostInitProperties();
    GlobalInputHandler = NewObject<UCommonGlobalInputHandler>(this);
}

void UCommonUISubsystem::NativeSetLegacyInputEnabled(bool bEnabled)
{
    if (!CommonInputManager && bEnabled)
    {
        CommonInputManager = NewObject<UCommonInputManager>(this);
        CommonInputManager->NativeOwnerSubsystem = this;
        // 14460bc60 validates the subsystem Outer as UGameInstance;
        // 144214d90 returns LocalPlayers[0], or null if the array is empty.
        UGameInstance* Instance = Cast<UGameInstance>(GetOuter());
        ULocalPlayer* Player = Instance->GetFirstGamePlayer();
        UCommonInputSubsystem* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
        CommonInputManager->NativeInputSubsystem = Input;
        CommonInputManager->GlobalInputHandler->NativeInputSubsystem = Input;
        CommonInputManager->NativeInputSuspensionChanged.AddUObject(this,
            &UCommonUISubsystem::NativeHandleLegacyInputSuspensionChanged);
        NativeLegacyInputManagerChanged.Broadcast(true);
    }
    else if (CommonInputManager && !bEnabled)
    {
        CommonInputManager->NativeInputSuspensionChanged.RemoveAll(this);
        CommonInputManager->MarkPendingKill();
        CommonInputManager = nullptr;
        NativeLegacyInputManagerChanged.Broadcast(false);
    }
}

void UCommonUISubsystem::NativeHandleLegacyInputSuspensionChanged(bool bSuspended)
{
    OnInputSuspensionChanged.Broadcast(bSuspended); // 1407c9550
}
