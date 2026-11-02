#include "CommonActionBindingProviders.h"
#include "Components/Widget.h"
#include "Engine/LocalPlayer.h"
#include "UCommonInputSubsystem.h"
#include "UCommonUIActionRouter.h"
#include "UCommonUIInputSettings.h"
#include "UObject/UObjectGlobals.h"

const FUIInputAction* FCommonActionBindingEngineResolver::FindTag(FName Tag)
{
    const UCommonUIInputSettings* Settings = GetDefault<UCommonUIInputSettings>();
    const FUIInputAction* Action = Settings->InputActions.FindByPredicate(
        [Tag](const FUIInputAction& Candidate) { return Candidate.ActionTag.GetTagName() == Tag; });
    return Action;
}

const FCommonInputActionData* FCommonActionBindingEngineResolver::FindRow(const FDataTableRowHandle& Row)
{
    return Row.GetRow<FCommonInputActionData>(TEXT(""));
}

const FUIInputAction& FCommonActionBindingEngineResolver::ResolveTag(FName Tag)
{
    return *FindTag(Tag);
}

const FCommonInputActionData& FCommonActionBindingEngineResolver::ResolveRow(const FDataTableRowHandle& Row)
{
    return *FindRow(Row);
}

int32 FCommonActionBindingEngineResolver::ResolveGamepadIndex(UObject* Owner)
{
    ULocalPlayer* Player = CastChecked<UWidget>(Owner)->GetOwningLocalPlayer();
    UCommonUIActionRouter* Router = Player ? Player->GetSubsystem<UCommonUIActionRouter>() : nullptr;
    UCommonInputSubsystem* Input = Router->GetLocalPlayer()->GetSubsystem<UCommonInputSubsystem>();
    return static_cast<uint8>(Input->GamepadInputType);
}
