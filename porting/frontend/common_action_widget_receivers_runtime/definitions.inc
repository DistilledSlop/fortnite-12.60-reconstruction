UCommonInputSubsystem* UCommonActionWidget::NativeResolveInputSubsystem() const
{
    const FCommonActionBindingRecord* Binding = CommonActionBindingData::Find(NativeBindingKey);
    UWidget* Owner = Binding ? static_cast<UWidget*>(Binding->Owner.Get()) : nullptr;
    ULocalPlayer* Player = Owner ? Owner->GetOwningLocalPlayer() : GetOwningLocalPlayer();
    return Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
}

void UCommonActionWidget::NativeBindInputMethodChanged(bool bEnable)
{
    UCommonInputSubsystem* Input = NativeResolveInputSubsystem();
    if (Input)
    {
        Input->NativeInputMethodChanged.RemoveAll(this);
        if (bEnable)
            Input->NativeInputMethodChanged.AddUObject(this, &UCommonActionWidget::NativeHandleInputMethodChanged);
    }
}

void UCommonActionWidget::NativeHandleInputMethodChanged(ECommonInputType InputType)
{
    NativeRefreshInputAction();
    OnInputMethodChanged.Broadcast(InputType == ECommonInputType::Gamepad);
}

void UCommonActionWidget::NativeSetBindingKey(int32 NewKey)
{
    if (NativeBindingKey == NewKey)
        return;
    if (!((NativeBindingKey >= 0 && CommonActionBindingData::Find(NativeBindingKey)) ||
          (NewKey >= 0 && CommonActionBindingData::Find(NewKey))))
        return;
    UCommonInputSubsystem* Input = NativeResolveInputSubsystem();
    if (Input)
        Input->NativeInputMethodChanged.RemoveAll(this);
    NativeBindingKey = NewKey;
    NativeBindInputMethodChanged(true);
}

void UCommonActionWidget::SetInputAction(FDataTableRowHandle InputActionRow)
{
    if (NativeBindingKey >= 0 && CommonActionBindingData::Find(NativeBindingKey))
    {
        UCommonInputSubsystem* Input = NativeResolveInputSubsystem();
        if (Input)
            Input->NativeInputMethodChanged.RemoveAll(this);
        NativeBindingKey = -1;
        NativeBindInputMethodChanged(true);
    }
    InputActions.Reset();
    InputActions.Add(InputActionRow);
    NativeRefreshInputAction();
}

void UCommonActionWidget::SetInputActions(TArray<FDataTableRowHandle> NewInputActions)
{
    if (NativeBindingKey >= 0 && CommonActionBindingData::Find(NativeBindingKey))
    {
        UCommonInputSubsystem* Input = NativeResolveInputSubsystem();
        if (Input)
            Input->NativeInputMethodChanged.RemoveAll(this);
        NativeBindingKey = -1;
        NativeBindInputMethodChanged(true);
    }
    InputActions = NewInputActions;
    NativeRefreshInputAction();
}

void UCommonActionWidget::NativeRefreshInputAction()
{
    if (!GetWorld())
        return;
    UCommonInputSubsystem* Input = NativeResolveInputSubsystem();
    if (!Input || !ShouldShowCommonInputKeys() || InputActions.Num() <= 0)
    {
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }
    const FCommonInputActionData* Row = InputActions[0].GetRow<FCommonInputActionData>(
        TEXT("CommonUIUtils::GetInputActionData couldn't find the row passed in, check data table if its missing."));
    if (!Row)
    {
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }
    if (NativeHidden)
    {
        SetVisibility(ESlateVisibility::Collapsed);
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }
    NativeCachedBrush = NativeGetIconForSubsystem(NativeResolveInputSubsystem());
    if (NativeCachedBrush.DrawAs == ESlateBrushDrawType::NoDrawType)
    {
        SetVisibility(ESlateVisibility::Collapsed);
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }
    if (!NativeIcon.IsValid())
    {
        SetVisibility(ESlateVisibility::Collapsed);
        return;
    }
    NativeIcon->SetImage(&NativeCachedBrush);
    const ECommonInputType Type = Input->GetCurrentInputType();
    const ECommonGamepadType Gamepad = Input->NativeGetGamepadInputType();
    const FCommonInputTypeInfo* Info = &Row->KeyboardInputTypeInfo;
    if (Type == ECommonInputType::Gamepad)
        Info = &Row->GamepadInputTypeInfos[static_cast<uint8>(Gamepad)];
    else if (Type == ECommonInputType::Touch)
        Info = &Row->TouchInputTypeInfo;
    NativeProgress->SetVisibility(Info->bActionRequiresHold ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed);
    NativeBox->Invalidate(EInvalidateWidgetReason::Layout | EInvalidateWidgetReason::Volatility);
    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

FText UCommonActionWidget::GetDisplayText() const
{
    UCommonInputSubsystem* Input = NativeResolveInputSubsystem();
    if (!Input || InputActions.Num() <= 0)
        return FText::GetEmpty();
    const FCommonInputActionData* Row = InputActions[0].GetRow<FCommonInputActionData>(
        TEXT("CommonUIUtils::GetInputActionData couldn't find the row passed in, check data table if its missing."));
    if (!Row)
        return FText::GetEmpty();
    const ECommonInputType Type = Input->GetCurrentInputType();
    const ECommonGamepadType Gamepad = Input->NativeGetGamepadInputType();
    const FCommonInputTypeInfo* Info = &Row->KeyboardInputTypeInfo;
    if (Type == ECommonInputType::Gamepad)
        Info = &Row->GamepadInputTypeInfos[static_cast<uint8>(Gamepad)];
    else if (Type == ECommonInputType::Touch)
        Info = &Row->TouchInputTypeInfo;
    return Info->bActionRequiresHold ? Row->HoldDisplayName : Row->DisplayName;
}

bool UCommonActionWidget::IsHeldAction() const
{
    UCommonInputSubsystem* Input = NativeResolveInputSubsystem();
    if (!Input || InputActions.Num() <= 0)
        return false;
    const FCommonInputActionData* Row = InputActions[0].GetRow<FCommonInputActionData>(
        TEXT("CommonUIUtils::GetInputActionData couldn't find the row passed in, check data table if its missing."));
    if (!Row)
        return false;
    const ECommonInputType Type = Input->GetCurrentInputType();
    const ECommonGamepadType Gamepad = Input->NativeGetGamepadInputType();
    const FCommonInputTypeInfo* Info = &Row->KeyboardInputTypeInfo;
    if (Type == ECommonInputType::Gamepad)
        Info = &Row->GamepadInputTypeInfos[static_cast<uint8>(Gamepad)];
    else if (Type == ECommonInputType::Touch)
        Info = &Row->TouchInputTypeInfo;
    return Info->bActionRequiresHold;
}

void UCommonActionWidget::SetIconRimBrush(FSlateBrush InIconRimBrush)
{
    IconRimBrush = InIconRimBrush;
}
