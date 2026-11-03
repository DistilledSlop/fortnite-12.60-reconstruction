// Includes: Components/DynamicEntryBox.h, Components/Image.h,
// Components/PanelWidget.h, Components/ScrollBox.h, Engine/LocalPlayer.h,
// UFortGlobalUIContext.h, UFortConfirmationWindow.h.

void UFortConfirmationWindow::NativeRemoveConfirmationButtons144f555e0()
{
    for (const FFortConfirmationButtonInfo& Info : ConfirmationButtons)
        if (Info.Button)
            Info.Button->NativeClicked.RemoveAll(this);
    if (DeclineButton)
        DeclineButton->NativeClicked.RemoveAll(this);
    ConfirmationButtons.Empty();
    DeclineButton = nullptr;
    EntryBox_SimpleButtons->Reset(true);
    EntryBox_ComplexChoiceButtons->Reset(true);
}

void UFortConfirmationWindow::NativeResetConfirmation144f75f90()
{
    ConfirmationDescription = FFortDialogDescription_NUI();
    NativeRemoveConfirmationButtons144f555e0();
    NativeRefreshConfirmationDisplay144f7b690();
}

void UFortConfirmationWindow::NativeRefreshConfirmationDisplay144f7b690()
{
    Panel_LeftOptionalWidget->ClearChildren();
    if (ConfirmationDescription.LeftAdditionalContent)
    {
        Panel_LeftOptionalWidget->AddChild(ConfirmationDescription.LeftAdditionalContent);
        Panel_LeftOptionalWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    else
        Panel_LeftOptionalWidget->SetVisibility(ESlateVisibility::Collapsed);

    Panel_OptionalWidget->ClearChildren();
    if (ConfirmationDescription.AdditionalContent)
    {
        Panel_OptionalWidget->AddChild(ConfirmationDescription.AdditionalContent);
        Panel_OptionalWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    else
        Panel_OptionalWidget->SetVisibility(ESlateVisibility::Collapsed);

    CommonTextBlock_Title->SetText(ConfirmationDescription.MessageHeader);
    CommonTextBlock_Description->SetText(ConfirmationDescription.MessageBody);
    ScrollBox_Description->SetVisibility(ConfirmationDescription.MessageBody.IsEmpty()
        ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
    Image_MainIcon->SetBrush(ConfirmationDescription.Icon);
    Image_MainIcon->SetVisibility(ConfirmationDescription.Icon.GetResourceObject()
        ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UFortConfirmationWindow::NativeCloseConfirmation144f55ae0()
{
    NativeRemoveConfirmationButtons144f555e0();
    if (!NativeConfirmationWindowClosed.IsBound())
        DeactivateWidget();
    else
        NativeConfirmationWindowClosed.Broadcast(this);
}

void UFortConfirmationWindow::NativeDismissConfirmation(EFortDialogResult Result, FName ActionName)
{
    if (!IsVisible())
        return;
    FFortDialogDescription_NUI Saved(ConfirmationDescription);
    NativeResetConfirmation144f75f90();
    NativeCloseConfirmation144f55ae0();

    ULocalPlayer* Player = GetOwningLocalPlayer();
    UFortGlobalUIContext* Context = Player ? Player->GetSubsystem<UFortGlobalUIContext>() : nullptr;
    if (Context)
    {
        FFortDialogExternalLatentActionHandle Handle;
        Handle.Handle = INDEX_NONE;
        Saved.NativeResultCallback.ExecuteIfBound(Result, ActionName);
        Saved.NativeAdvancedResultCallback.ExecuteIfBound(Result, ActionName, false, Handle);
    }
}

void UFortConfirmationWindow::NativeClearConfirmation144f664f0()
{
    NativeDismissConfirmation(EFortDialogResult::Killed);
}

void NativeClearConfirmation144f664f0(UFortConfirmationWindow* Window)
{
    Window->NativeClearConfirmation144f664f0();
}

void UFortConfirmationWindow::BotClickButton()
{
    if (!NativeBotControllerEligible144f54030())
        return;
    if (ConfirmationDescription.ConfirmButtonInputActions.Num() > 0 && ConfirmationButtons.Num() > 0)
        NativeConfirmationButtonClicked144f5fbc0(ConfirmationButtons[0].Button);
    else
        NativeDismissConfirmation(EFortDialogResult::Declined);
}

void UFortConfirmationWindow::NativeConfirmationButtonClicked144f5fbc0(UCommonButton* Button)
{
    if (!Button || !IsVisible())
        return;
    FName ActionName = NAME_None;
    for (const FFortConfirmationButtonInfo& Info : ConfirmationButtons)
    {
        if (Info.Button == Button)
        {
            ActionName = Info.NativeActionName;
            break;
        }
    }
    if (ConfirmationDescription.bShouldWaitForLatentActionOnConfirmAction)
        NativeConfirmLatent144f5fbc0(Button, ActionName);
    else
        NativeDismissConfirmation(EFortDialogResult::Confirmed, ActionName);
}

void UFortConfirmationWindow::NativeConfirmLatent144f5fbc0(UCommonButton* Button, FName ActionName)
{
    Button->SetIsEnabled(false);
    ULocalPlayer* Player = GetOwningLocalPlayer();
    UFortGlobalUIContext* Context = Player ? Player->GetSubsystem<UFortGlobalUIContext>() : nullptr;
    if (!Context)
        return;
    NativeDialogLatentHandle.Handle = INDEX_NONE;
    NativeAllocateDialogLatentHandle141f87900(NativeDialogLatentHandle);
    ConfirmationDescription.NativeResultCallback.ExecuteIfBound(EFortDialogResult::Confirmed, ActionName);
    const FFortDialogExternalLatentActionHandle Handle = NativeDialogLatentHandle;
    ConfirmationDescription.NativeAdvancedResultCallback.ExecuteIfBound(
        EFortDialogResult::Confirmed, ActionName, true, Handle);
    EntryBox_SimpleButtons->Reset(true);
    NativeSetLoadGuardLoading1407f7e20(CommonLoadGuard_LatentSpinner, true);
}

bool UFortConfirmationWindow::NativeBotControllerEligible144f54030() const
{
    const AFortPlayerController* Controller = Cast<AFortPlayerController>(GetOwningPlayer());
    return Controller && Controller->IsClientBot();
}

void UFortConfirmationWindow::NativeDeclineButtonClicked144f5fce0(UCommonButton*)
{
    NativeDismissConfirmation(EFortDialogResult::Declined);
}

void UFortConfirmationWindow::NativeHandleActivated()
{
    NativeRemoveConfirmationButtons144f555e0();
    const bool bSimple = ConfirmationDescription.ConfirmButtonInputActions.Num() <= 1;
    for (const FConfirmationDialogAction& Action : ConfirmationDescription.ConfirmButtonInputActions)
    {
        UDynamicEntryBox* Box = bSimple ? EntryBox_SimpleButtons : EntryBox_ComplexChoiceButtons;
        if (UCommonButton* Button = Box->CreateEntry<UCommonButton>())
        {
            Button->SetIsEnabled(true);
            ConfigureConfirmationButton(Button, Action, bSimple);
            Button->NativeClicked.AddUObject(this, &UFortConfirmationWindow::NativeConfirmationButtonClicked144f5fbc0, Button);
            FFortConfirmationButtonInfo& Info = ConfirmationButtons.AddDefaulted_GetRef();
            Info.Button = Button;
            Info.NativeActionName = Action.ResultName;
            if (bSimple)
                Button->SetNavigationRuleExplicit(EUINavigation::Down, nullptr);
        }
    }
    UCommonButton* NewDecline = nullptr;
    if (ConfirmationDescription.DeclineButtonInputAction != NAME_None)
    {
        NewDecline = EntryBox_SimpleButtons->CreateEntry<UCommonButton>();
        if (NewDecline)
        {
            ConfigureDeclineButton(NewDecline);
            NewDecline->NativeClicked.AddUObject(this, &UFortConfirmationWindow::NativeDeclineButtonClicked144f5fce0, NewDecline);
        }
    }
    DeclineButton = NewDecline;
    NativeRefreshConfirmationDisplay144f7b690();
    Super::NativeHandleActivated();
    NativeSetLoadGuardLoading1407f7e20(CommonLoadGuard_LatentSpinner, false);
    UWidget* FocusTarget = this;
    for (const FFortConfirmationButtonInfo& Info : ConfirmationButtons)
        if (Info.Button) { FocusTarget = Info.Button; break; }
    NativeConfirmationFocus144f6bdb9(this, FocusTarget);
    NativeConfirmationScroll144e1c6e0(ScrollBox_Description);
}
