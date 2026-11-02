#pragma once

#include "CoreMinimal.h"
#include "ECommonInputType.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerInput.h"

class UFortPlayerInput;
class UFortPlayerInputSettings;
class UFortClientSettingsRecord;
class APlayerController;

// Descriptive source aliases for the closed action-settings and binding-match
// phases. They are not an alternative keyboard processor or controller policy.
namespace FortKeyboardBindingHelpers
{
    // 142390950 and the requested-type prefix1423812cc..142381311. Count is
    // preserved when no real CommonInput subsystem exists; Touch is not remapped.
    FORTNITEGAME_API bool IsUsingGamepad(const APlayerController& Controller);
    FORTNITEGAME_API ECommonInputType ResolveRequestedInputType(const APlayerController& Controller,
        ECommonInputType RequestedType);
    // 14200e000. Append unique mappings without clearing the caller's array.
    FORTNITEGAME_API bool CollectActionMappings(const UFortPlayerInputSettings& Settings,
        FName Action, TArray<FInputActionKeyMapping>& OutMappings);
    // 14200df90. The first matching alias wins; InputActionType is not a gate.
    FORTNITEGAME_API bool FindActionKeyAlias(const UFortPlayerInputSettings& Settings,
        FName Action, FKey& OutKey);
    // Actual reflected pointer getters 141fe7d30 / 141fe7d00.
    FORTNITEGAME_API UFortPlayerInputSettings* GetDesiredKeyboardSettings(const UFortPlayerInput& Input);
    FORTNITEGAME_API UFortPlayerInputSettings* GetDesiredGamepadSettings(const UFortPlayerInput& Input);
    // Settings-mapping branches of1423812b0. Empty results leave outputs intact.
    FORTNITEGAME_API bool SelectSettingsActionKeys(const UFortPlayerInputSettings& Settings,
        FName Action, bool bGamepadSettings, FKey& Primary, FKey& Secondary);
    // Engine fallback1423815cd..1423816c1. Retains all matching mapping copies
    // while selecting the first two, just as the original temporary array does.
    FORTNITEGAME_API bool SelectEngineActionKeys(const UPlayerInput& Input,
        FName Action, ECommonInputType RequestedType, FKey& Primary, FKey& Secondary);
    // 141e32a40 uses the existing reflected CloudFileState, not a new live flag.
    FORTNITEGAME_API bool CanUseClientKeyboardSettings(const UFortClientSettingsRecord& Settings);
    // Match/execute phase of14236b4d0 AFTER1423812b0 resolved its two output keys.
    // This does not claim that the unresolved controller resolver is supplied.
    FORTNITEGAME_API bool MatchResolvedActionBinding(FInputActionBinding& Binding,
        FKey Key, EInputEvent Event, bool bKeysResolved,
        const FKey& Primary, const FKey& Secondary, bool bExecute);
}
