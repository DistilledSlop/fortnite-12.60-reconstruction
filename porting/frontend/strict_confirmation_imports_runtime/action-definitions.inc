// Required includes: Blueprint/UserWidget.h, Engine/LocalPlayer.h,
// Engine/World.h, Engine/GameInstance.h, UFortLocalPlayer.h,
// UCommonUISubsystem.h, UCommonInputManager.h, UFortUIManagerInterface.h,
// AFortPlayerController.h, UFortGameInstance.h, and ConfirmationImportProviders.h.

UFortAsyncAction_ShowConfirmation_NUI*
UFortAsyncAction_ShowConfirmation_NUI::ShowSimpleConfirmationDialog_CustomInput(
    UObject* InWorldContextObject, FSlateBrush Icon, FText Title, FText Message,
    FName ConfirmAction, FName DeclineAction, UWidget* AdditionalContent,
    UWidget* LeftAdditionalContent, bool Cancelable, ULocalPlayer* TargetPlayer)
{
    UFortAsyncAction_ShowConfirmation_NUI* Action = NewObject<UFortAsyncAction_ShowConfirmation_NUI>(GetTransientPackage());
    Action->WorldContextObject = InWorldContextObject;
    Action->TargetLocalPlayer = Cast<UFortLocalPlayer>(TargetPlayer);
    Action->Description.Icon = Icon;
    Action->Description.MessageHeader = Title;
    Action->Description.MessageBody = Message;
    if (ConfirmAction != NAME_None)
    {
        FConfirmationDialogAction& Confirm = Action->Description.ConfirmButtonInputActions.Emplace_GetRef();
        Confirm.ActionName = ConfirmAction;
    }
    Action->Description.DeclineButtonInputAction = DeclineAction;
    Action->Description.AdditionalContent = AdditionalContent;
    Action->Description.LeftAdditionalContent = LeftAdditionalContent;
    Action->Description.Cancelable = Cancelable;
    if (UCommonInputManager* Manager = NativeConfirmationFindInputManager14514f020(InWorldContextObject))
    {
        if (Manager->NativeIsExternalInputSuspended())
        {
            Action->bNativeRestoreExternalInputSuspension = true;
            Manager->NativeSetExternalInputSuspended(false);
        }
    }
    return Action;
}

void UFortAsyncAction_ShowConfirmation_NUI::Activate()
{
    const FNativeConfirmationManagerPair* Pair = NativeConfirmationResolveManager14514eda0(WorldContextObject);
    if (!Pair || !Pair->Object || !Pair->Interface)
    {
        NativeOnConfirmationResult(EFortDialogResult::Unknown, NAME_None);
        return;
    }
    if (WorldContextObject && !TargetLocalPlayer)
    {
        if (UUserWidget* Widget = Cast<UUserWidget>(WorldContextObject))
            TargetLocalPlayer = Cast<UFortLocalPlayer>(Widget->GetOwningLocalPlayer());
        else if (UWorld* World = WorldContextObject->GetWorld())
        {
            if (UFortGameInstance* Instance = Cast<UFortGameInstance>(World->GetGameInstance()))
            {
                AFortPlayerController* Controller = Cast<AFortPlayerController>(Instance->GetPrimaryPlayerController());
                TargetLocalPlayer = Cast<UFortLocalPlayer>(Cast<ULocalPlayer>(Controller->Player));
            }
        }
    }
    Description.NativeResultCallback.BindUObject(this, &UFortAsyncAction_ShowConfirmation_NUI::NativeOnConfirmationResult);
    Pair->Interface->NativeShowConfirmation(TargetLocalPlayer, Description);
}

void UFortAsyncAction_ShowConfirmation_NUI::NativeOnConfirmationResult(EFortDialogResult Result, FName ResultName)
{
    if (bNativeRestoreExternalInputSuspension)
    {
        if (UCommonInputManager* Manager = NativeConfirmationFindInputManager14514f020(WorldContextObject))
            Manager->NativeSetExternalInputSuspended(true);
        bNativeRestoreExternalInputSuspension = false;
    }
    DialogResult.Broadcast(Result, ResultName);
}
