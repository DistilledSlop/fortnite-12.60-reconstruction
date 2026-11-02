namespace
{
const FSlateNoResource& CommonActionWidgetNoResource()
{
    static const FSlateNoResource NoResource;
    return NoResource;
}
}

FSlateBrush UCommonActionWidget::GetIcon() const
{
    const FCommonActionBindingRecord* Binding = CommonActionBindingData::Find(NativeBindingKey);
    UWidget* Owner = Binding ? static_cast<UWidget*>(Binding->Owner.Get()) : nullptr;
    ULocalPlayer* Player = Owner ? Owner->GetOwningLocalPlayer() : GetOwningLocalPlayer();
    UCommonInputSubsystem* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;

    return NativeGetIconForSubsystem(Input);
}

FSlateBrush UCommonActionWidget::NativeGetIconForSubsystem(UCommonInputSubsystem* Input) const
{
    TArray<FKey> Keys;
    for (const FDataTableRowHandle& Handle : InputActions)
    {
        const FCommonInputActionData* Row = Handle.GetRow<FCommonInputActionData>(
            TEXT("CommonUIUtils::GetInputActionData couldn't find the row passed in, check data table if its missing."));
        if (!Row)
        {
            return CommonActionWidgetNoResource();
        }
        const FCommonInputTypeInfo* Info = &Row->KeyboardInputTypeInfo;
        if (Input)
        {
            const ECommonInputType Type = Input->GetCurrentInputType();
            const ECommonGamepadType Gamepad = Input->NativeGetGamepadInputType();
            if (Type == ECommonInputType::Gamepad)
                Info = &Row->GamepadInputTypeInfos[static_cast<uint8>(Gamepad)];
            else if (Type == ECommonInputType::Touch)
                Info = &Row->TouchInputTypeInfo;
        }
        Keys.Add(Info->Key);
    }
    FSlateBrush Brush;
    const ECommonGamepadType Gamepad = Input->NativeGetGamepadInputType();
    const ECommonInputType Type = Input->GetCurrentInputType();
    const FCommonInputPlatformData Platform = GetCommonInputSettings()->NativeGetCurrentPlatformData();
    if (FindCommonInputPlatformBrush(Platform, Keys, Type, Gamepad, Brush))
        return Brush;
    return CommonActionWidgetNoResource();
}
