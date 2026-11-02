#include "FortKeyboardBindingHelpers.h"
#include "UFortPlayerInput.h"
#include "UFortPlayerInputSettings.h"
#include "UFortClientSettingsRecord.h"
#include "UCommonInputSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

static_assert(sizeof(FInputActionBinding) == 0x48, "Original controller input-record type");
static_assert(sizeof(FInputActionKeyMapping) == 0x28, "Original action-settings mapping type");

bool FortKeyboardBindingHelpers::IsUsingGamepad(const APlayerController& Controller)
{
    const auto* Player = Controller.GetLocalPlayer();
    const auto* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
    return Input && Input->GetCurrentInputType() == ECommonInputType::Gamepad;
}
ECommonInputType FortKeyboardBindingHelpers::ResolveRequestedInputType(const APlayerController& Controller,
    ECommonInputType RequestedType)
{
    if (RequestedType == ECommonInputType::Count)
    {
        const auto* Player = Controller.GetLocalPlayer();
        const auto* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
        if (Input) RequestedType = Input->GetCurrentInputType();
    }
    return RequestedType;
}

bool FortKeyboardBindingHelpers::CollectActionMappings(const UFortPlayerInputSettings& Settings,
    FName Action, TArray<FInputActionKeyMapping>& OutMappings)
{
    for (const auto& Mapping : Settings.ActionMappings)
        if (Mapping.ActionName == Action) OutMappings.AddUnique(Mapping);
    // Original1420060a0 equality covers action FName, key FName and all four
    // modifier bits, matching FInputActionKeyMapping::operator== in this Engine.
    return OutMappings.Num() > 0;
}
bool FortKeyboardBindingHelpers::FindActionKeyAlias(const UFortPlayerInputSettings& Settings,
    FName Action, FKey& OutKey)
{
    for (const auto& Alias : Settings.ActionKeyAliases)
        if (Alias.ActionName == Action) { OutKey = Alias.KeyAlias; return true; }
    return false;
}
UFortPlayerInputSettings* FortKeyboardBindingHelpers::GetDesiredKeyboardSettings(const UFortPlayerInput& Input)
{
    return Input.DesiredKBMPlayerInputSettings;
}
UFortPlayerInputSettings* FortKeyboardBindingHelpers::GetDesiredGamepadSettings(const UFortPlayerInput& Input)
{
    return Input.DesiredGamepadPlayerInputSettings;
}
bool FortKeyboardBindingHelpers::SelectSettingsActionKeys(const UFortPlayerInputSettings& Settings,
    FName Action, bool bGamepadSettings, FKey& Primary, FKey& Secondary)
{
    TArray<FInputActionKeyMapping> Mappings;
    const bool bFound = CollectActionMappings(Settings, Action, Mappings);
    if (bFound)
    {
        Primary = Mappings[0].Key;
        // The gamepad profile branch copies only the first mapping. Multiple
        // mappings merely produce a diagnostic in that branch, not a second key.
        if (!bGamepadSettings && Mappings.Num() > 1) Secondary = Mappings[1].Key;
    }
    return bFound;
}
bool FortKeyboardBindingHelpers::SelectEngineActionKeys(const UPlayerInput& Input,
    FName Action, ECommonInputType RequestedType, FKey& Primary, FKey& Secondary)
{
    TArray<FInputActionKeyMapping> Mappings;
    for (const auto& Mapping : Input.GetKeysForAction(Action))
    {
        const bool bMatch = RequestedType == ECommonInputType::Gamepad
            ? Mapping.Key.IsGamepadKey()
            : RequestedType == ECommonInputType::MouseAndKeyboard
                && !Mapping.Key.IsGamepadKey() && !Mapping.Key.IsTouch();
        if (bMatch) Mappings.Add(Mapping);
    }
    if (Mappings.Num() > 0) Primary = Mappings[0].Key;
    if (Mappings.Num() > 1) Secondary = Mappings[1].Key;
    return Mappings.Num() > 0;
}
bool FortKeyboardBindingHelpers::CanUseClientKeyboardSettings(const UFortClientSettingsRecord& Settings)
{
    return Settings.CloudFileState == ECloudFileState::Saving || Settings.CloudFileState == ECloudFileState::Idle;
}
bool FortKeyboardBindingHelpers::MatchResolvedActionBinding(FInputActionBinding& Binding,
    FKey Key, EInputEvent Event, bool bKeysResolved,
    const FKey& Primary, const FKey& Secondary, bool bExecute)
{
    if (Binding.KeyEvent != Event || !bKeysResolved || (Key != Primary && Key != Secondary)) return false;
    if (bExecute) Binding.ActionDelegate.Execute(Key);
    // A match returns true even when the unified delegate is unbound or this
    // was a query-only call. FInputBinding.bConsumeInput is not inspected here.
    return true;
}
